/**
 * @file dac.hpp
 * @author Chris DeFrancisci (chrisdefrancisci@gmail.com)
 * @brief
 * @date 2026-02-10
 */

#pragma once

#include <span>

#include <reusable_synth/hardware/interrupt_handler.hpp>

/**
 * @brief
 *
 * @todo documentation
 * @todo switch params so everything is out, in
 *
 * @tparam ComputationType
 * @tparam DacType
 * @tparam inputSize
 */
template<typename ComputationType, typename DacType, size_t InputSize>
class Dac
{
public:
    /**
     * @brief Construct a new Dac object
     *
     * @param inputData
     * @param outputData
     * @param convert
     */
    Dac(std::span<ComputationType, InputSize> inputData,
        std::span<DacType, InputSize * 2> outputData,
        void (*convert)(DacType& out, const ComputationType& in))
      : inputData(inputData)
      , outputData(outputData)
      , convert(convert)
      , callbackFlag(CallbackType::FullComplete) // init to start writing data
                                                 // in the middle
    {
    }

    /**
     * @brief Copies data to the DAC buffer.
     *
     */
    void execute()
    {
        if (callbackFlag == CallbackType::HalfComplete) {
            callbackFlag = CallbackType::None;
            auto inIter = inputData.begin();
            auto outIter = outputData.begin();
            auto inEnd = inIter + inputData.size();
            auto outEnd = outIter + inputData.size();
            for (; inIter != inEnd && outIter != outEnd; inIter++, outIter++) {
                convert(*outIter, *inIter);
            }
        } else if (callbackFlag == CallbackType::FullComplete) {
            callbackFlag = CallbackType::None;
            auto inIter = inputData.begin();
            auto outIter = outputData.begin() + inputData.size();
            auto inEnd = inputData.end();
            auto outEnd = outputData.end();
            for (; inIter != inEnd && outIter != outEnd; inIter++, outIter++) {
                convert(*outIter, *inIter);
            }
        }
    }

    /**
     * @brief Ready to receive new data - call to prevent creating data faster
     * than the DAC is outputting it.
     *
     * @return true If data written to inputData will be copied to outputData.
     * @return false If data written to inputData will not be copied to
     * outputData.
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
    std::span<ComputationType, InputSize> inputData;
    std::span<DacType, InputSize * 2> outputData;

    enum class CallbackType : int
    {
        None,
        HalfComplete,
        FullComplete
    };
    void (*convert)(DacType& out, const ComputationType& in);
    CallbackType callbackFlag;
};

// Deduction guide, see
// https://stackoverflow.com/questions/40951697/what-are-template-deduction-guides-and-when-should-we-use-them
// https://en.cppreference.com/w/cpp/language/class_template_argument_deduction.html
template<typename ComputationType, typename DacType, size_t InputSize>
Dac(std::array<ComputationType, InputSize>,
    std::array<DacType, InputSize * 2>,
    void (*convert)(DacType& out, const ComputationType& in))
  -> Dac<ComputationType, DacType, InputSize>;

template<typename ComputationType, typename DacType, size_t InputSize>
Dac(std::span<ComputationType, InputSize>,
    std::array<DacType, InputSize * 2>,
    void (*convert)(DacType& out, const ComputationType& in))
  -> Dac<ComputationType, DacType, InputSize>;

template<typename ComputationType, typename DacType, size_t InputSize>
Dac(std::array<ComputationType, InputSize>,
    std::span<DacType, InputSize * 2>,
    void (*convert)(DacType& out, const ComputationType& in))
  -> Dac<ComputationType, DacType, InputSize>;
