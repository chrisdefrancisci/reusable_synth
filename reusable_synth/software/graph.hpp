/**
 * @file graph.hpp
 * @author Chris DeFrancisci (chrisdefrancisci@gmail.com)
 * @brief Contains classes to implement a graph in order to sort vertices by
 * dependencies.
 * @date 2026-06-24
 */

#pragma once

#include <array>
#include <functional>
#include <memory_resource>
#include <optional>
#include <ranges>
#include <span>
#include <utility>
#include <vector>

#include <reusable_synth/software/vertex_interface.hpp>
#include <reusable_synth/utils/noncopyable.hpp>

/**
 * @brief A Graph owns / manages the lifetime of a collection of vertices.
 *
 * @tparam DataType Type used by the nodes
 * @tparam Size Size of the data used by the nodes.
 */
template<typename DataType, size_t Size>
class Graph
{
public:
    using VertexType = VertexInterface<DataType, Size>;
    using VertexPtr = VertexType*;

    /**
     * @brief Construct a new Graph object
     *
     * @param mem Memory resource where the vertices, adjacency list, etc. will
     * live.
     */
    explicit Graph(std::pmr::memory_resource* mem)
      : adjacencyList(mem)
      , mem(mem)
    {
    }

    ~Graph()
    {
        for (auto& vertex : std::ranges::reverse_view(adjacencyList)) {
            vertex->~VertexInterface();
        }
    }

    Graph(const Graph&) = delete;
    Graph(Graph&&) = delete;
    auto operator=(const Graph&) -> Graph& = delete;
    auto operator=(Graph&&) -> Graph& = delete;

    /**
     * @brief Adds a vertex of type T.
     *
     * @tparam T Vertex type.
     * @tparam Args Vertex constructor argument types.
     * @param args Vertex constructor arguments, excluding the vertex position.
     * @return size_t The position of the vertex in the underlying adjacency
     * list.
     */
    template<typename T, typename... Args>
    auto addVertex(Args&&... args) -> size_t
    {
        size_t pos = adjacencyList.size();
        adjacencyList.push_back(
          make_vertex<T>(mem, pos, std::forward<Args>(args)...));
        return pos;
    }

    /**
     * @brief Helper to access vertex in the adjacencyList.
     *
     * Does not check bounds.
     *
     * @param pos Position to access.
     * @return VertexPtr Pointer to the vertex.
     */
    [[nodiscard]] auto operator[](size_t pos) const -> VertexPtr
    {
        return adjacencyList[pos];
    }

    [[nodiscard]] auto size() const -> size_t { return adjacencyList.size(); }

    /**
     * @brief Adds a directed edge between to vertices.
     *
     * @param producer The vertex that produces data.
     * @param consumer The vertex that consumes data.
     * @param consumerInput The consumer's callback, which connects the
     * producer's output to (one of) the consumer's input(s).
     */
    void addEdge(size_t producer,
                 size_t consumer,
                 VertexType::ConnectFunc consumerInput)
    {
        addEdge(
          adjacencyList[producer], adjacencyList[consumer], consumerInput);
    }

private:
    void addEdge(VertexPtr producer,
                 VertexPtr consumer,
                 VertexType::ConnectFunc consumerInput)
    {
        producer->addConsumer(consumer, consumerInput);
    }
    std::pmr::vector<VertexPtr> adjacencyList;
    std::pmr::memory_resource* mem;
};

/**
 * @brief Computes a topologically sorted ordering of vertices in the graph.
 *
 * @see <a
 href="https://en.wikipedia.org/wiki/Topological_sorting#Depth-first_search">
 Depth First Search</a>
 * @tparam DataType Graph datatype
 * @tparam Size Vertex sizes
 * @param graph The graph to sort.
 * @param mem Memory resource to hold the sorted vector.
 * @return std::pmr::vector<VertexInterface<DataType, Size>*> Sorted vertices.
 */
template<typename DataType, size_t Size>
auto topological_sort(const Graph<DataType, Size>& graph,
                      std::pmr::memory_resource* mem)
  -> std::pmr::vector<VertexInterface<DataType, Size>*>
{
    enum class Mark
    {
        Unmarked,
        Temporary,
        Permanent
    };
    std::pmr::vector<Mark> marked{ mem };
    marked.resize(graph.size());
    for (auto& vertex : marked) {
        vertex = Mark::Unmarked;
    }

    std::pmr::vector<VertexInterface<DataType, Size>*> sorted{ mem };
    sorted.reserve(graph.size());

    auto visit = [&marked, &sorted, &graph](this auto self,
                                            size_t vertex) -> bool {
        if (marked[vertex] == Mark::Permanent) {
            return true;
        }
        // This occurs if a graph has a cycle, return error
        if (marked[vertex] == Mark::Temporary) {
            return false;
        }
        marked[vertex] = Mark::Temporary;
        for (auto dependents : graph[vertex]->getConsumers()) {
            if (!self(dependents)) {
                // Propagate failures out
                return false;
            }
        }

        // We have reached a vertex that either has no dependents or all
        // dependents have already been added. Add it to the start of the
        // list.
        marked[vertex] = Mark::Permanent;
        sorted.insert(sorted.begin(), graph[vertex]);

        return true;
    };

    auto getUnmarkedVertices = [&marked]() -> std::optional<size_t> {
        for (auto [idx, vertex] : std::views::enumerate(marked)) {
            if (vertex != Mark::Permanent) {
                return std::make_optional(idx);
            }
        }
        return std::nullopt;
    };

    while (auto vertex = getUnmarkedVertices()) {
        auto success = visit(vertex.value());
        if (!success) {
            sorted.clear();
            return sorted;
        }
    }
    return sorted;
}