/**
 * @file graph.hpp
 * @author Chris DeFrancisci (chrisdefrancisci@gmail.com)
 * @brief Contains classes to implement a graph in order to sort nodes by
 * dependencies.
 * @date 2026-06-24
 */

#pragma once

#include <array>
#include <functional>
#include <memory_resource>
#include <span>
#include <vector>

template<typename DataType, size_t Size>
class VertexInterface
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