#include <cmath>
#include <functional>
#include <array>
#include <memory_resource>
#include <ranges>
#include <span>

#include <gmock/gmock.h>
#include <gtest/gtest.h>
#include <reusable_synth/software/vertex_interface.hpp>
#include <tests/fakes/fake_vertices.hpp>

using namespace testing;

/** Computation type */
using Type = float;
/** Computation buffer size */
constexpr int size = 10;

TEST(VertexTest, ExecutesDependentVertices)
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
