/**
 * @file circular_dma.hpp
 * @author Chris DeFrancisci (chrisdefrancisci@gmail.com)
 * @brief
 * @date 2026-02-10
 */

#pragma once

#include <algorithm>
#include <span>

#include <reusable_synth/hardware/interrupt_handler.hpp>

enum class DmaDirection
{
    MemoryToPeripheral,
    PeripheralToMemory
};

/**
 * @brief Provides the frame work for a continuous, circular transfer of data.
 *
 * For a memory to peripheral transfer, source will be the memory and
 * destination will be the peripheral.
 *
 * @todo: will source = memory for peripheral to memory transfer? If so, this
 * should be renamed.
 *
 * @tparam T The type of the destination data.
 * @tparam Size The size of the internal memory data, or half the size of the
 * peripheral data.
 */
template<typename T, size_t Size>
class CircularDma
{
public:
    /**
     * @brief Construct a new CircularDma object
     *
     * @param memoryData
     * @param periphData
     */
    CircularDma(std::span<T, Size> memoryData,
                std::span<T, Size * 2> periphData)
      : memoryData(memoryData)
      , periphData(periphData)
      , callbackFlag(CallbackType::FullComplete) // init to start writing data
                                                 // in the middle
    {
    }

    /**
     * @brief Copies data to the CircularDma buffer.
     *
     */
    template<DmaDirection Direction>
    void execute()
    {
        if (callbackFlag == CallbackType::HalfComplete) {
            callbackFlag = CallbackType::None;
            auto periphBegin = periphData.begin();
            auto memBegin = memoryData.begin();
            auto periphEnd = periphBegin + Size;
            auto memEnd = memoryData.end();
            if constexpr (Direction == DmaDirection::MemoryToPeripheral) {
                std::copy(memBegin, memEnd, periphBegin);
            } else if constexpr (Direction ==
                                 DmaDirection::PeripheralToMemory) {
                std::copy(periphBegin, periphEnd, memBegin);
            }
        } else if (callbackFlag == CallbackType::FullComplete) {
            callbackFlag = CallbackType::None;
            auto periphBegin = periphData.begin() + Size;
            auto memBegin = memoryData.begin();
            auto periphEnd = periphData.end();
            auto memEnd = memoryData.end();
            if constexpr (Direction == DmaDirection::MemoryToPeripheral) {
                std::copy(memBegin, memEnd, periphBegin);
            } else if constexpr (Direction ==
                                 DmaDirection::PeripheralToMemory) {
                std::copy(periphBegin, periphEnd, memBegin);
            }
        }
    }

    /**
     * @brief Ready to receive new data - call to prevent creating data faster
     * than the CircularDma is outputting it.
     *
     * @return true If data written to periphData will be copied to memoryData.
     * @return false If data written to periphData will not be copied to
     * memoryData.
     */
    auto isReady() -> bool { return callbackFlag != CallbackType::None; }

    /**
     * @brief Function to call in DMA half complete IRQ handler.
     *
     */
    void setHalfCompleteFlag()
    {
        if (callbackFlag != CallbackType::None) {
            // TODO: Error! We are outputting data faster than we can create it
        } else {
            callbackFlag = CallbackType::HalfComplete;
        }
    }

    /**
     * @brief Function to call in DMA complete IRQ handler.
     *
     */
    void setCompleteFlag()
    {
        if (callbackFlag != CallbackType::None) {
            // TODO: Error! We are outputting data faster than we can create it
        } else {
            callbackFlag = CallbackType::FullComplete;
        }
    }

private:
    std::span<T, Size> memoryData;
    std::span<T, Size * 2> periphData;

    enum class CallbackType : int
    {
        None,
        HalfComplete,
        FullComplete
    };
    CallbackType callbackFlag;
};

// Deduction guide, see
// https://stackoverflow.com/questions/40951697/what-are-template-deduction-guides-and-when-should-we-use-them
// https://en.cppreference.com/w/cpp/language/class_template_argument_deduction.html
template<typename T, size_t Size>
CircularDma(std::array<T, Size>, std::array<T, Size * 2>)
  -> CircularDma<T, Size>;

template<typename T, size_t Size>
CircularDma(std::array<T, Size>, std::span<T, Size * 2>)
  -> CircularDma<T, Size>;

template<typename T, size_t Size>
CircularDma(std::span<T, Size>, std::array<T, Size * 2>)
  -> CircularDma<T, Size>;
