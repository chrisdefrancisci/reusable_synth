#include <array>
#include <cmath>
#include <functional>
#include <memory_resource>
#include <ranges>
#include <span>

#include <gmock/gmock.h>
#include <gtest/gtest.h>
#include <reusable_synth/software/graph.hpp>
#include <tests/fakes/fake_vertices.hpp>

using namespace testing;

/** Computation type */
using Type = float;
/** Computation buffer size */
constexpr int size = 10;

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