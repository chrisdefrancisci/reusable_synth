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
template<typename ComputationType, typename DacType, size_t inputSize>
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
    Dac(std::span<ComputationType, inputSize> inputData,
        std::span<DacType, inputSize * 2> outputData,
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
            auto in = inputData.begin();
            auto out = outputData.begin();
            auto inEnd = in + inputData.size();
            auto outEnd = out + inputData.size();
            for (; in != inEnd && out != outEnd; in++, out++) {
                convert(*out, *in);
            }
        } else if (callbackFlag == CallbackType::FullComplete) {
            callbackFlag = CallbackType::None;
            auto in = inputData.begin();
            auto out = outputData.begin() + inputData.size();
            auto inEnd = inputData.end();
            auto outEnd = outputData.end();
            for (; in != inEnd && out != outEnd; in++, out++) {
                convert(*out, *in);
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
    inline bool isReady() { return callbackFlag != CallbackType::None; }

    /**
     * @brief Function to call in DMA half complete IRQ handler.
     *
     */
    inline void setHalfCompleteFlag()
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
    inline void setCompleteFlag()
    {
        if (callbackFlag != CallbackType::None) {
            // TODO: Error! We are outputting data faster than we can create it
        } else {
            callbackFlag = CallbackType::FullComplete;
        }
    }

private:
    std::span<ComputationType, inputSize> inputData;
    std::span<DacType, inputSize * 2> outputData;

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
template<typename ComputationType, typename DacType, size_t inputSize>
Dac(std::array<ComputationType, inputSize>,
    std::array<DacType, inputSize * 2>,
    void (*convert)(DacType& out, const ComputationType& in))
  -> Dac<ComputationType, DacType, inputSize>;

template<typename ComputationType, typename DacType, size_t inputSize>
Dac(std::span<ComputationType, inputSize>,
    std::array<DacType, inputSize * 2>,
    void (*convert)(DacType& out, const ComputationType& in))
  -> Dac<ComputationType, DacType, inputSize>;

template<typename ComputationType, typename DacType, size_t inputSize>
Dac(std::array<ComputationType, inputSize>,
    std::span<DacType, inputSize * 2>,
    void (*convert)(DacType& out, const ComputationType& in))
  -> Dac<ComputationType, DacType, inputSize>;
