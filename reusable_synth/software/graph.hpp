/**
 * @file graph.hpp
 * @author Chris DeFrancisci (chrisdefrancisci@gmail.com)
 * @brief Contains classes to implement a graph in order to sort nodes by
 * dependencies.
 * @date 2026-06-24
 */

#pragma once

#include "reusable_synth/utils/noncopyable.hpp"

#include <array>
#include <functional>
#include <memory_resource>
#include <optional>
#include <ranges>
#include <span>
#include <utility>
#include <vector>

/**
 * @brief
 *
 * @todo: The way this is currently structured takes a ton of memory. Should
 * look into an algorithm to determine how many simultaneous computation buffers
 * are needed.
 * @tparam DataType
 * @tparam Size
 */
template<typename DataType, size_t Size>
class VertexInterface : Noncopyable
{
public:
    using ConnectFunc = void (VertexInterface<DataType, Size>::*)(
      std::span<const DataType, Size>);

    VertexInterface(int id, std::pmr::memory_resource* mem)
      : id(id)
      , consumers(mem)
    {
    }

    virtual ~VertexInterface() = default;

    /**
     * @brief Get a non-owning view to the vertex's output buffer
     *
     * @return std::span<DataType, Size> Output buffer
     */
    [[nodiscard]] auto getOutput() -> std::span<const DataType, Size>
    {
        return std::span(outputBuff);
    }

    /**
     * @brief Get the ID of the vertex object
     *
     * @return int ID
     */
    [[nodiscard]] auto getId() const -> int { return id; }

    /**
     * @brief Provides the output buffer of this algorithm to be used as an
     * algorithm input.
     *
     * In the Directed Acyclic Graph, each consumer is a vertex with a directed
     * edge \a from this vertex \a to the consumer vertices.
     *
     * @param other A vertex dependent on this vertex.
     * @param connect Function that registers the output buffer of this vertex
     * as an input to the consumer vertex.
     */
    void addConsumer(VertexInterface<DataType, Size>* other,
                     ConnectFunc connect)
    {
        std::invoke(connect, other, getOutput());
        consumers.push_back(other->id);
    }

    /**
     * @brief Get the vector of vertices that consume this vertex's output.
     *
     * @return const std::pmr::vector<int>& Vector of dependent vertices.
     */
    [[nodiscard]] auto getConsumers() const -> const std::pmr::vector<int>&
    {
        return consumers;
    }

    /**
     * @brief Returns the list of functions that will connect a buffer to an
     * algorithm input.
     *
     * @return std::pmr::vector<ConnectFunc>
     */
    virtual auto getInputs() const -> const std::pmr::vector<ConnectFunc>& = 0;

    /**
     * @brief Executes the algorithm associated with this vertex, reading
     * input data and writing it to the output buffer.
     *
     */
    virtual void execute() = 0;

protected:
    /**
     * @brief Get a non-owning view to the vertex's output buffer
     *
     * @return std::span<DataType, Size> Output buffer
     */
    [[nodiscard]] auto getOutput(size_t i) -> DataType&
    {
        return outputBuff[i];
    }

private:
    int id;
    std::array<DataType, Size> outputBuff{};
    std::pmr::vector<int> consumers;
};

/**
 * @brief Helper function to create a vertex in the memory resource.
 *
 * All this really does is enforce the pattern of the vertex arguments called
 * first followed by the pmr argument.
 *
 * @tparam T The object type (i.e., something derived from VertexInterface)
 * @tparam Args Argument types to T's constructor
 * @param mem The memory buffer resource
 * @param args Arguments to T's constructor
 * @return T* Pointer to the new object
 */
template<typename T, typename... Args>
auto make_vertex(std::pmr::memory_resource* mem, Args&&... args) -> T*
{
    std::pmr::polymorphic_allocator<> alloc{ mem };
    return alloc.new_object<T>(std::forward<Args>(args)..., mem);
}

template<typename DataType, size_t Size>
class Graph : Noncopyable
{
public:
    using VertexType = VertexInterface<DataType, Size>;
    using VertexPtr = VertexType*;
    explicit Graph(std::pmr::memory_resource* mem)
      : adjacencyList(mem)
      , mem(mem)
    {
    }

    template<typename T, typename... Args>
    auto addVertex(Args&&... args) -> size_t
    {
        size_t pos = adjacencyList.size();
        adjacencyList.push_back(
          make_vertex<T>(mem, pos, std::forward<Args>(args)...));
        return pos;
    }

    [[nodiscard]] auto operator[](size_t pos) const -> VertexPtr
    {
        return adjacencyList[pos];
    }

    [[nodiscard]] auto size() const -> size_t { return adjacencyList.size(); }

    void addEdge(size_t producer, size_t consumer, VertexType::ConnectFunc func)
    {
        addEdge(adjacencyList[producer], adjacencyList[consumer], func);
    }

private:
    void addEdge(VertexPtr producer,
                 VertexPtr consumer,
                 VertexType::ConnectFunc func)
    {
        producer->addConsumer(consumer, func);
    }
    std::pmr::vector<VertexPtr> adjacencyList;
    std::pmr::memory_resource* mem;
};

/**
 * @brief Computes
 *
 * @see <a
 href="https://en.wikipedia.org/wiki/Topological_sorting#Depth-first_search">
 Depth First Search</a>
 * @tparam DataType
 * @tparam Size
 * @param graph
 * @param mem
 * @return std::pmr::vector<VertexInterface<DataType, Size>*>
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
        // dependents have already been added. Add it to the start of the list.
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