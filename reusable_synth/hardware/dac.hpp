/**
 * @file dac.hpp
 * @author Chris DeFrancisci (chrisdefrancisci@gmail.com)
 * @brief
 * @date 2026-02-10
 */

#pragma once

#include <span>

#include <reusable_synth/hardware/interrupt_handler.hpp>

template<typename ComputationType, typename DacType, size_t inputSize>
class Dac
{
public:
    Dac(std::span<ComputationType, inputSize> inputData,
        std::span<DacType, inputSize * 2> outputData,
        void (*convertAndCheck)(DacType& out, const ComputationType& in))
      : inputData(inputData)
      , outputData(outputData)
      , convertAndCheck(convertAndCheck)
      , callbackFlag(CallbackType::FullComplete) // init to start writing data
                                                 // in the middle
    {
        // TODO: ensure input data and output data are the same size
    }
    void execute()
    {
        if (callbackFlag == CallbackType::HalfComplete) {
            callbackFlag = CallbackType::None;
            auto in = inputData.begin();
            auto out = outputData.begin();
            auto inEnd = in + inputData.size() / 2;
            auto outEnd = out + inputData.size() / 2;
            for (; in != inEnd && out != outEnd; in++, out++) {
                convertAndCheck(*out, *in);
            }
        } else if (callbackFlag == CallbackType::FullComplete) {
            callbackFlag = CallbackType::None;
            auto in = inputData.begin() + inputData.size() / 2;
            auto out = outputData.begin() + inputData.size() / 2;
            auto inEnd = inputData.end();
            auto outEnd = outputData.end();
            for (; in != inEnd && out != outEnd; in++, out++) {
                convertAndCheck(*out, *in);
            }
        } else {
            // TODO: Error! we are creating data faster than it can be output
        }
    }

    inline void setHalfCompleteFlag()
    {
        if (callbackFlag != CallbackType::None) {
            // TODO: Error! We are outputting data faster than we can create it
        } else {
            callbackFlag = CallbackType::HalfComplete;
        }
    }
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
    CallbackType callbackFlag;
    void (*convertAndCheck)(DacType& out, const ComputationType& in);
};

// Deduction guide, see
// https://stackoverflow.com/questions/40951697/what-are-template-deduction-guides-and-when-should-we-use-them
// https://en.cppreference.com/w/cpp/language/class_template_argument_deduction.html
template<typename ComputationType, typename DacType, size_t inputSize>
Dac(std::array<ComputationType, inputSize>,
    std::array<DacType, inputSize * 2>,
    void (*convertAndCheck)(DacType& out, const ComputationType& in))
  -> Dac<ComputationType, DacType, inputSize>;
