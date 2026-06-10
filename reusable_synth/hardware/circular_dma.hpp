/**
 * @file circular_dma.hpp
 * @author Chris DeFrancisci (chrisdefrancisci@gmail.com)
 * @brief
 * @date 2026-02-10
 */

#pragma once

#include <algorithm>
#include <cstddef>
#include <ranges>
#include <span>
#include <type_traits>

#include <reusable_synth/hardware/interrupt_handler.hpp>

enum class DmaDirection
{
    MemoryToPeripheral,
    PeripheralToMemory
};

/**
 * @brief Provides the framework for a continuous, circular transfer of data.
 *
 * For a memory to peripheral transfer, source will be the memory and
 * destination will be the peripheral.
 *
 * @todo: enforce that Size is divisible by NChannels
 *
 * @tparam T The type of the destination data.
 * @tparam Size The size of the all memory data, or half
 * the size of the peripheral data.
 * @tparam NChannels The number of channels to be interleaved to the peripheral
 * or deinterleaved from the peripheral
 */
template<DmaDirection Direction, int NChannels, typename T, size_t Size>
class CircularDma
{
public:
    using enum DmaDirection;
    /**
     * @brief Construct a new CircularDma object
     *
     * @param memoryData
     * @param periphData
     */
    CircularDma(std::span<T, Size> memoryData,
                std::span<T, static_cast<long>(Size) * 2> periphData)
      : memoryData(memoryData)
      , periphData(periphData)
      , callbackFlag(CallbackType::FullComplete) // init to start writing data
                                                 // in the middle
    {
        if constexpr (Direction == MemoryToPeripheral) {
            callbackFlag = CallbackType::FullComplete;
        } else if constexpr (Direction == PeripheralToMemory) {
            callbackFlag = CallbackType::None;
        }
    }

    /**
     * @brief Copies data to the CircularDma buffer.
     *
     */
    void execute()
    {
        constexpr auto channelBufSize = Size / NChannels;
        using InputType = std::conditional_t<Direction == MemoryToPeripheral,
                                             decltype(memoryData.begin()),
                                             decltype(periphData.begin())>;
        using OutputType = std::conditional_t<Direction == MemoryToPeripheral,
                                              decltype(periphData.begin()),
                                              decltype(memoryData.begin())>;
        InputType inputBeginIt;
        InputType inputEndIt;
        OutputType outputBeginIt;

        // Set iterators
        if (callbackFlag == CallbackType::HalfComplete) {
            if constexpr (Direction == MemoryToPeripheral) {
                inputBeginIt = memoryData.begin();
                inputEndIt = memoryData.end();
                outputBeginIt = periphData.begin();
            } else if constexpr (Direction == PeripheralToMemory) {
                inputBeginIt = periphData.begin();
                inputEndIt = periphData.begin() + Size;
                outputBeginIt = memoryData.begin();
            }
            callbackFlag = CallbackType::None;
        } else if (callbackFlag == CallbackType::FullComplete) {
            if constexpr (Direction == MemoryToPeripheral) {
                inputBeginIt = memoryData.begin();
                inputEndIt = memoryData.end();
                outputBeginIt = periphData.begin() + Size;
            } else if constexpr (Direction == PeripheralToMemory) {
                inputBeginIt = periphData.begin() + Size;
                inputEndIt = periphData.end();
                outputBeginIt = memoryData.begin();
            }
            callbackFlag = CallbackType::None;
        }

        // Do copy
        if constexpr (NChannels == 1) {
            std::copy(inputBeginIt, inputEndIt, outputBeginIt);
        } else {
            // Mem -> Periph: Need to interleave
            // Periph -> Mem: Need to deinterleave
            // TODO: maybe make interleave/deinterleave iterators?
            if constexpr (Direction == MemoryToPeripheral) {
                // TODO: make as below (and refactor)
                for (int channel = 0; channel < NChannels; channel++) {
                    auto channelBuf =
                      std::span(&*(inputBeginIt + (channel * channelBufSize)),
                                channelBufSize);
                    for (size_t sample = 0; sample < channelBufSize; sample++) {
                        *(outputBeginIt + (sample * NChannels) + channel) =
                          channelBuf[sample];
                    }
                }
            } else if constexpr (Direction == PeripheralToMemory) {
                // TODO: refactor for cleanliness
                for (int channel = 0; channel < NChannels; channel++) {
                    auto channelBuf =
                      std::span(&*(outputBeginIt + (channel * channelBufSize)),
                                channelBufSize);
                    for (size_t sample = 0; sample < channelBufSize; sample++) {
                        channelBuf[sample] =
                          *(inputBeginIt + (sample * NChannels) + channel);
                    }
                }
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

template<DmaDirection Direction, typename T, size_t Size>
auto make_circulardma(std::span<T, Size> memoryData,
                      std::span<T, static_cast<long>(Size) * 2> periphData)
  -> CircularDma<Direction, 1, T, Size>
{
    return CircularDma<Direction, 1, T, Size>(memoryData, periphData);
}

template<DmaDirection Direction, int NChannels, typename T, size_t Size>
auto make_circulardma(std::span<T, Size> memoryData,
                      std::span<T, static_cast<long>(Size) * 2> periphData)
  -> CircularDma<Direction, NChannels, T, Size>
{
    return CircularDma<Direction, NChannels, T, Size>(memoryData, periphData);
}