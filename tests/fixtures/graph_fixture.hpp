/**
 * @file graph_fixture.hpp
 * @author Chris DeFrancisci (chrisdefrancisci@gmail.com)
 * @brief Helper test fixture for tests that require a graph. Note that all the
 * functionality of the fixture is already proved out in
 * tests/software/graph_test.cpp
 * @date 2026-08-24
 */

#include <array>
#include <cmath>
#include <functional>
#include <memory_resource>
#include <ranges>
#include <span>

#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include <reusable_synth/software/graph.hpp>

template<typename T, size_t Size>
class GraphFixture : public testing::Test // NOLINT(*naming)
{
protected:
    GraphFixture()
      : mbr(arena.data(), arena.size(), std::pmr::null_memory_resource())
      , graph(&mbr)
    {
    }

private:
    std::array<uint8_t, 8194> arena{};
    std::pmr::monotonic_buffer_resource mbr;

protected:
    Graph<T, Size> graph; // NOLINT(*variable*)
    void executeSorted()
    {
        auto sorted = topological_sort(graph, &mbr);

        for (auto* vertex : sorted) {
            vertex->execute();
        }
    }
};