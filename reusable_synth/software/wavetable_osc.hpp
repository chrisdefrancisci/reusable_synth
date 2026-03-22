/**
 * @file wavetable_osc.hpp
 * @author Chris DeFrancisci (chrisdefrancisci@gmail.com)
 * @brief
 * @date 2026-02-03
 */

#pragma once

#include <cmath>
#include <numbers>
#include <span>

/**
 * @brief Creates a compile-time square wave.
 *
 * @remarks This is inappropriate for audio due to aliasing, need virtual analog
 * square wave.
 *
 * @tparam T Underlying data type
 * @tparam Period Number of samples to form one period
 * @tparam min Min value of the square
 * @tparam max Max value of the square
 */
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

/**
 * @brief Creates a buffer with a ramp waveform
 *
 * @todo Is making the waveform constructor constexpr actually helpful?
 * @tparam T Underlying data type
 * @tparam Period Number of samples to form one period
 * @tparam min Min value of the ramp
 * @tparam max (Hypothetical) max value of the ramp sampled at the discontinuity
 */
template<typename T, int Period, int min = -1, int max = 1>
struct RampWavetable
{
    constexpr RampWavetable()
      : data()
    {
        float step = float(max - min) / float(Period);
        for (int i = 0; i < Period; i++) {
            data[i] = min + T(i * step);
        }
    }
    T data[Period];
};

/**
 * @brief Creates a buffer with a sinusoidal waveform
 *
 * @todo could make constexpr sin if I install https://github.com/kthohr/gcem
 * @todo replace min max scaling the next time I need it
 * @tparam T Underlying data type
 * @tparam Period Number of samples to form one period
 * @tparam min Min value of the sine wave
 * @tparam max Max value of the sine wave
 */
template<typename T, int Period, int min = -1, int max = 1>
struct SineWavetable
{
    constexpr SineWavetable()
      : data()
    {
        float step = float(max - min) / float(Period);
        for (int i = 0; i < Period; i++) {
            float raw = std::sin(std::numbers::pi_v<float> * 2.0f * i / Period);
            data[i] = T((raw + 1.0f) * float(max - min) / 2.0f + min);
        }
    }
    T data[Period];
};

// TODO define constexpr virtual analog table creation

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
        delta = float(wavetable.size()) * frequency / sampleRate;
    }

    void increment(T& out)
    {
        auto index0 = (unsigned int)fractionalIndex;
        auto index1 =
          index0 + 1 == wavetable.size() ? (unsigned int)0 : index0 + 1;
        float frac = fractionalIndex - float(index0);
        out =
          wavetable[index0] + frac * (wavetable[index1] - wavetable[index0]);
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

template<typename T>
WavetableOsc(const T*) -> WavetableOsc<T>;