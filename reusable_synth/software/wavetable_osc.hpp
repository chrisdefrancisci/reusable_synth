/**
 * @file wavetable_osc.hpp
 * @author Chris DeFrancisci (chrisdefrancisci@gmail.com)
 * @brief
 * @date 2026-02-03
 */

#pragma once

#include <span>

// TODO: ensure template is even
// see https://learnmoderncpp.com/2023/09/03/concepts-101/
template<typename T, int Period, int min = -1, int max = 1>
struct SquareWavetable
{
    constexpr SquareWavetable()
      : data()
    {
        T value = min;
        for (auto i = 0, duty = 0; i != Period; i++, duty++) {
            if (duty == Period / 2) {
                duty = 0;
                if (value == min) {
                    value = max;
                } else {
                    value = min;
                }
            }
            data[i] = value;
        }
    }
    T data[Period];
};

// TODO: if first sample == last sample, we can optimize checking the indices
// see https://juce.com/tutorials/tutorial_wavetable_synth/
template<typename T>
class WavetableOsc
{
public:
    explicit WavetableOsc(const std::span<const T> wavetable)
      : wavetable(wavetable)
      , fractionalIndex(0.0)
      , delta(0)
    {
    }

    void setFrequency(float frequency, float sampleRate)
    {
        delta = frequency * float(wavetable.size()) / sampleRate;
    }

    void increment(T& out)
    {
        auto index0 = (unsigned int)fractionalIndex;
        auto index1 =
          index0 + 1 == wavetable.size() ? (unsigned int)0 : index0 + 1;
        float dx = float(index1) - fractionalIndex;
        T dy = wavetable[index1] - wavetable[index0];
        out = wavetable[index0] + dy / dx;
        fractionalIndex += delta;
        if (fractionalIndex >= wavetable.size()) {
            fractionalIndex -= wavetable.size();
        }
    }

    void increment(std::span<T> out)
    {
        for (auto& sample : out) {
            increment(sample);
        }
    }

private:
    const std::span<const T> wavetable;
    float fractionalIndex;
    float delta;
};
