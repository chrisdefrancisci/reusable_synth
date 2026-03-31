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
 * @tparam Min Min value of the square
 * @tparam Max Max value of the square
 */
template<typename T, int Period, int Min = -1, int Max = 1>
struct SquareWavetable
{
    constexpr SquareWavetable()
      : data()
    {
        T value = Min;
        for (auto i = 0, duty = 0; i != Period; i++, duty++) {
            if (duty == Period / 2) {
                duty = 0;
                if (value == Min) {
                    value = Max;
                } else {
                    value = Min;
                }
            }
            data[i] = value;
        }
    }
    std::array<T, Period> data;
};

/**
 * @brief Creates a buffer with a ramp waveform
 *
 * @todo Is making the waveform constructor constexpr actually helpful?
 * @tparam T Underlying data type
 * @tparam Period Number of samples to form one period
 * @tparam Min Min value of the ramp
 * @tparam Max (Hypothetical) max value of the ramp sampled at the discontinuity
 */
template<typename T, int Period, int Min = -1, int Max = 1>
struct RampWavetable
{
    constexpr RampWavetable()
      : data()
    {
        float step = float(Max - Min) / float(Period);
        for (int i = 0; i < Period; i++) {
            data[i] = Min + T((float)i * step);
        }
    }
    std::array<T, Period> data;
};

/**
 * @brief Creates a buffer with a sinusoidal waveform
 *
 * @todo could make constexpr sin if I install https://github.com/kthohr/gcem
 * @todo replace min max scaling the next time I need it
 * @tparam T Underlying data type
 * @tparam Period Number of samples to form one period
 * @tparam Min Min value of the sine wave
 * @tparam Max Max value of the sine wave
 */
template<typename T, int Period, int Min = -1, int Max = 1>
struct SineWavetable
{
private:
    static constexpr float sineAmplitude = 2.0;

public:
    constexpr SineWavetable()
      : data()
    {
        for (int i = 0; i < Period; i++) {
            float raw = std::sin(std::numbers::pi_v<float> * sineAmplitude *
                                 (float)i / Period);
            data[i] =
              T(((raw + 1.0F) * float(Max - Min) / sineAmplitude) + Min);
        }
    }
    std::array<T, Period> data;
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
        out = wavetable[index0] +
              (frac * (float(wavetable[index1]) - float(wavetable[index0])));
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
    std::span<const T> wavetable;
    float fractionalIndex{};
    float delta{};
};

template<typename T>
WavetableOsc(const T*) -> WavetableOsc<T>;

template<typename T, size_t N>
WavetableOsc(std::array<T, N>) -> WavetableOsc<T>;