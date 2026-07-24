/**
 * @file adc.hpp
 * @author Chris DeFrancisci (chrisdefrancisci@gmail.com)
 * @brief
 * @date 2026-07-19
 */

#include <algorithm>
#include <memory_resource>

#include <ranges>
#include <reusable_synth/software/vertex_interface.hpp>

/**
 * @brief This is mostly just a wrapper for a buffer of data
 *
 * @tparam DataType
 * @tparam Size
 */
template<typename HardwareType, typename DataType, size_t Size>
class AnalogIn : VertexInterface<DataType, Size>
{
public:
    using typename VertexInterface<DataType, Size>::ConnectFunc;

    AnalogIn(int id,
             std::span<const HardwareType, Size> otherBuffer,
             DataType (*func)(HardwareType),
             std::pmr::memory_resource* mem)
      : VertexInterface<DataType, Size>(id, mem)
      , otherBuffer(otherBuffer)
      , func(func)
    {
    }

    /**
     * @brief Get the empty object - should not dynamically select input buffer
     *
     * @return std::span<const ConnectFunc>
     */
    [[nodiscard]] auto getInputs() const
      -> std::span<const ConnectFunc> override
    {
        return std::span<const ConnectFunc>{}; // Should be empty
    }

    void execute() override
    {
        for (size_t i = 0; i < Size; i++) {
            std::ranges::transform(otherBuffer, this->getOutputBegin(), func);
        }
    }

private:
    std::span<const HardwareType, Size> otherBuffer;
    DataType (*func)(HardwareType);
};