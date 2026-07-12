#include "gmock/gmock.h"
#include "gtest/gtest.h"
#include <cmath>
#include <functional>
#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include <array>
#include <memory_resource>
#include <ranges>
#include <span>

#include <reusable_synth/software/graph.hpp>

using namespace testing;

/** Computation type */
using Type = float;
/** Computation buffer size */
constexpr int size = 10;

template<typename DataType, size_t Size>
class InitialVertex : public VertexInterface<DataType, Size>
{
public:
    using typename VertexInterface<DataType, Size>::ConnectFunc;
    InitialVertex(int id,
                  std::function<void(DataType&)> init,
                  std::pmr::memory_resource* mem)
      : VertexInterface<DataType, Size>(id, mem)
    {
        for (int i = 0; i < Size; i++) {
            init(this->getOutput(i));
        }
    }

    [[nodiscard]] auto getInputs() const
      -> const std::pmr::vector<ConnectFunc>& override
    {
        return inputs; // Should be empty
    }

    /**
     * @brief No-op
     *
     */
    void execute() override {}

private:
    std::pmr::vector<ConnectFunc> inputs;
};

template<typename DataType, size_t Size>
class DoubleInput : public VertexInterface<DataType, Size>
{
public:
    using typename VertexInterface<DataType, Size>::ConnectFunc;

    DoubleInput(int id, std::pmr::memory_resource* mem)
      : VertexInterface<DataType, Size>(id, mem)
    {
        inputs.push_back(
          static_cast<ConnectFunc>(&DoubleInput<DataType, Size>::input));
    }

    [[nodiscard]] auto getInputs() const
      -> const std::pmr::vector<ConnectFunc>& override
    {
        return inputs;
    }

    void execute() override
    {
        ASSERT_EQ(inputBuff.size(), Size);
        for (size_t i = 0; i < Size; i++) {
            this->getOutput(i) = inputBuff[i] * DataType(2);
        }
    }

private:
    std::span<const DataType>
      inputBuff{}; // Fixed size span does not have default ctor
    void input(std::span<const DataType, Size> outputBuff)
    {
        // I believe we need to static cast here because this will be invoked
        // using a VertexInterface pointer
        static_cast<DoubleInput<DataType, Size>*>(this)->inputBuff = outputBuff;
    }
    std::pmr::vector<ConnectFunc> inputs;
};

template<typename DataType, size_t Size>
class MultiplyInputs : public VertexInterface<DataType, Size>
{
public:
    using typename VertexInterface<DataType, Size>::ConnectFunc;

    static constexpr size_t connectA = 0;
    static constexpr size_t connectB = 1;

    MultiplyInputs(int id, std::pmr::memory_resource* mem)
      : VertexInterface<DataType, Size>(id, mem)
    {
        inputs.push_back(
          static_cast<ConnectFunc>(&MultiplyInputs<DataType, Size>::inputA));
        inputs.push_back(
          static_cast<ConnectFunc>(&MultiplyInputs<DataType, Size>::inputB));
    }

    [[nodiscard]] auto getInputs() const
      -> const std::pmr::vector<ConnectFunc>& override
    {
        return inputs;
    }

    void execute() override
    {
        ASSERT_EQ(inputABuff.size(), Size);
        ASSERT_EQ(inputBBuff.size(), Size);
        for (size_t i = 0; i < Size; i++) {
            this->getOutput(i) = inputABuff[i] * inputBBuff[i];
        }
    }

private:
    std::span<const DataType>
      inputABuff{}; // Fixed size span does not have default ctor
    std::span<const DataType>
      inputBBuff{}; // Fixed size span does not have default ctor

    void inputA(std::span<const DataType, Size> outputBuff)
    {
        static_cast<MultiplyInputs<DataType, Size>*>(this)->inputABuff =
          outputBuff;
    }
    void inputB(std::span<const DataType, Size> outputBuff)
    {
        static_cast<MultiplyInputs<DataType, Size>*>(this)->inputBBuff =
          outputBuff;
    }

    std::pmr::vector<ConnectFunc> inputs;
};

TEST(GraphTest, ExecutesDependentVertices)
{
    // In this case, we do not create a whole graph and instead just use vector
    // of vertices directly
    std::array<uint8_t, 8194> arena{};
    std::pmr::monotonic_buffer_resource mbr(
      arena.data(), arena.size(), std::pmr::null_memory_resource());
    std::pmr::polymorphic_allocator<> pmr(&mbr);

    // Create two vertices
    std::pmr::vector<VertexInterface<Type, size>*> vec(&mbr);
    vec.reserve(10);
    std::pmr::polymorphic_allocator<> alloc{ &mbr };
    vec.push_back(make_vertex<DoubleInput<Type, size>>(&mbr, vec.size()));
    vec.push_back(make_vertex<DoubleInput<Type, size>>(&mbr, vec.size()));

    // At the start, neither vertex should have consumers
    ASSERT_EQ(vec[0]->getConsumers().size(), 0);
    ASSERT_EQ(vec[1]->getConsumers().size(), 0);

    // Connect them with an edge going from 0 -> 1
    vec[0]->addConsumer(vec[1], vec[1]->getInputs().back());

    // Now, vertex 0 should have 1 consumer: vertex with ID 1
    ASSERT_EQ(vec[0]->getConsumers().size(), 1);
    ASSERT_EQ(vec[1]->getConsumers().size(), 0);
    EXPECT_EQ(vec[0]->getConsumers().back(), 1);

    // Create an input buffer for vertex 0 to read from
    std::array<Type, size> inputBuff{};
    std::invoke(vec[0]->getInputs().back(), vec[0], inputBuff);

    // Initialize inputBuff
    for (size_t i = 0; i < inputBuff.size(); i++) {
        inputBuff.at(i) = (float)i * 5;
    }

    // Execute the vertices
    for (auto& vertex : vec) {
        vertex->execute();
    }

    // Both vertices output buffers should now be equal to the input buffer time
    // some multiplier
    // First vertex's buffer should be x2
    // Second vertex's buffer should be x2 x2 = x4
    constexpr float floatNear = 0.001;
    EXPECT_THAT(vec[0]->getOutput(),
                Pointwise(Truly([](const std::tuple<Type, Type>& p) -> bool {
                              auto [value, expected] = p;
                              expected *= 2.0F;
                              return std::abs(value - expected) < floatNear;
                          }),
                          inputBuff));

    EXPECT_THAT(vec[1]->getOutput(),
                Pointwise(Truly([](const std::tuple<Type, Type>& p) -> bool {
                              auto [value, expected] = p;
                              expected *= 4.0F;
                              return std::abs(value - expected) < floatNear;
                          }),
                          inputBuff));
    // Destroy objects in reverse order, unsure if it really matters
    for (auto& vertex : std::ranges::reverse_view(vec)) {
        vertex->~VertexInterface();
    }
}

TEST(GraphTest, SortSingleInput)
{
    std::array<uint8_t, 8194> arena{};
    std::pmr::monotonic_buffer_resource mbr(
      arena.data(), arena.size(), std::pmr::null_memory_resource());
    Graph<Type, size> graph(&mbr);

    // Connect each vertex to the previously created vertex
    for (int i = 0; i < 10; ++i) {
        graph.addVertex<DoubleInput<Type, size>>();
        if (i > 0) {
            graph.addEdge(i, i - 1, graph[i - 1]->getInputs().back());
        }
    }

    // Special case for last-added vertex (first topologically)
    graph.addVertex<InitialVertex<Type, size>>(
      std::function<void(Type&)>([](Type& item) -> void { item = Type(1.0); }));
    graph.addEdge(graph.size() - 1,
                  graph.size() - 2,
                  graph[graph.size() - 2]->getInputs().back());

    auto sorted = topological_sort(graph, &mbr);

    for (int i = 0; i < graph.size(); ++i) {
        EXPECT_EQ(sorted[i], graph[graph.size() - 1 - i]);
    }

    for (auto* vertex : sorted) {
        vertex->execute();
    }

    for (auto [i, vertex] : std::views::enumerate(sorted)) {
        EXPECT_THAT(vertex->getOutput(), Each(std::pow(2.0F, i)));
    }
}

TEST(GraphTest, SortDoubleInput)
{
    // Create a graph with this structure:
    // Const1 = 1.0F
    //   |
    //   v
    // Double1   Const2 = 3.0F
    //   |       /     |
    //   v      v      v
    //  Multiply1   Double2
    //       |       /
    //       v      v
    //      Multiply2
    //

    std::array<uint8_t, 8194> arena{};
    std::pmr::monotonic_buffer_resource mbr(
      arena.data(), arena.size(), std::pmr::null_memory_resource());
    Graph<Type, size> graph(&mbr);
    // Create nodes in an order that definitely isn't right
    auto mult2Idx = graph.addVertex<MultiplyInputs<Type, size>>();
    auto mult1Idx = graph.addVertex<MultiplyInputs<Type, size>>();
    auto const1Idx = graph.addVertex<InitialVertex<Type, size>>(
      std::function<void(Type&)>([](Type& item) -> void { item = Type(1.0); }));
    auto const2Idx = graph.addVertex<InitialVertex<Type, size>>(
      std::function<void(Type&)>([](Type& item) -> void { item = Type(3.0); }));
    auto double1Idx = graph.addVertex<DoubleInput<Type, size>>();
    auto double2Idx = graph.addVertex<DoubleInput<Type, size>>();

    // Create edges
    // Edges from left side
    graph.addEdge(const1Idx, double1Idx, graph[double1Idx]->getInputs().back());
    graph.addEdge(
      double1Idx,
      mult1Idx,
      graph[mult1Idx]->getInputs()[MultiplyInputs<Type, size>::connectA]);
    graph.addEdge(
      mult1Idx,
      mult2Idx,
      graph[mult2Idx]->getInputs()[MultiplyInputs<Type, size>::connectA]);
    // Middle edge
    graph.addEdge(
      const2Idx,
      mult1Idx,
      graph[mult1Idx]->getInputs()[MultiplyInputs<Type, size>::connectB]);
    // Edges from right side
    graph.addEdge(const2Idx, double2Idx, graph[double2Idx]->getInputs().back());
    graph.addEdge(
      double2Idx,
      mult2Idx,
      graph[mult2Idx]->getInputs()[MultiplyInputs<Type, size>::connectB]);

    auto sorted = topological_sort(graph, &mbr);

    for (auto* vertex : sorted) {
        vertex->execute();
    }

    EXPECT_THAT(graph[const1Idx]->getOutput(), Each(Type(1.0)));
    EXPECT_THAT(graph[const2Idx]->getOutput(), Each(Type(3.0)));
    EXPECT_THAT(graph[double1Idx]->getOutput(), Each(Type(2.0)));
    EXPECT_THAT(graph[mult1Idx]->getOutput(), Each(Type(6.0)));
    EXPECT_THAT(graph[double2Idx]->getOutput(), Each(Type(6.0)));
    EXPECT_THAT(graph[mult2Idx]->getOutput(), Each(Type(36.0)));
}