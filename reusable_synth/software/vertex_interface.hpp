/**
 * @file vertex_interface.hpp
 * @author Chris DeFrancisci (chrisdefrancisici@gmail.com)
 * @brief
 * @date 2026-07-12
 */

#pragma once

#include <array>
#include <functional>
#include <memory_resource>
#include <optional>
#include <span>
#include <utility>
#include <vector>

#include "reusable_synth/utils/noncopyable.hpp"

/**
 * @brief Abstract interface for wrapping algorithms.
 *
 * Used by the graph class to ensure algorithms with dependencies are executed
 * after their dependencies.
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
     * @todo move getOutput() into fake_vertices - there's no real-world use
     * case for being able to arbitrarily peek into the buffer.
     * @return std::span<DataType, Size> Output buffer
     */
    [[nodiscard]] auto getOutput() const -> std::span<const DataType, Size>
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
    [[nodiscard]] auto getConsumers() const -> std::span<const int>
    {
        return std::span<const int>(consumers);
    }

    /**
     * @brief Returns the list of functions that will connect a buffer to an
     * algorithm input.
     *
     * @return std::pmr::vector<ConnectFunc>
     */
    virtual auto getInputs() const -> std::span<const ConnectFunc> = 0;

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
     * @todo delete in VertexInterface::outputBuff refactor
     * @return std::span<DataType, Size> Output buffer
     */
    [[nodiscard]] auto getOutput(size_t i) -> DataType&
    {
        return outputBuff[i];
    }

    /**
     * @brief Get the outputBuff.begin()
     *
     * @todo delete in VertexInterface::outputBuff refactor
     * @return std::array<Datatype, Size>::iterator
     */
    auto getOutputBegin() -> std::array<Datatype, Size>::iterator
    {
        return outputBuff.begin();
    }

private:
    int id;
    // TODO: mandating that each implementation has to have its own array is
    // terrible
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
