/**
 * @file wavetable_osc.hpp
 * @author Chris DeFrancisci (chrisdefrancisci@gmail.com)
 * @brief
 * @date 2026-02-03
 */

#pragma once

#include <cmath>
#include <numbers>
#include <optional>
#include <span>

/**
 * @brief Creates a compile-time square wave.
 *
 * @remarks This is inappropriate for audio due to aliasing, need virtual analog
 * square wave.
 *
 * @tparam T Underlying data type
 * @tparam Period Number of samples to form one period
 */
template<typename T, int Period>
struct SquareWavetable
{

    /**
     * @brief Constructs the object and fills the wavetable buffer.
     *
     * @param min Min value of the square
     * @param max Max value of the square
     */
    constexpr SquareWavetable(T min, T max)
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
    std::array<T, Period> data;
};

/**
 * @brief Creates a buffer with a ramp waveform
 *
 * @todo Is making the waveform constructor constexpr actually helpful?
 * @tparam T Underlying data type
 * @tparam Period Number of samples to form one period
 */
template<typename T, int Period>
struct RampWavetable
{

    /**
     * @brief Constructs the object and fills the wavetable buffer.
     *
     * @param min Min value of the ramp
     * @param max (Hypothetical) max value of the ramp sampled at the
     * discontinuity
     */
    constexpr RampWavetable(T min, T max)
      : data()
    {
        float step = float(max - min) / float(Period);
        for (int i = 0; i < Period; i++) {
            data[i] = min + T((float)i * step);
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
 */
template<typename T, int Period>
struct SineWavetable
{
private:
    static constexpr float sineAmplitude = 2.0;

public:
    /**
     * @brief Constructs the object and fills the wavetable buffer.
     *
     * @param min Min value of the sine wave
     * @param max Max value of the sine wave
     */
    constexpr SineWavetable(T min, T max)
      : data()
    {
        for (int i = 0; i < Period; i++) {
            float raw = std::sin(std::numbers::pi_v<float> * sineAmplitude *
                                 (float)i / Period);
            data[i] =
              T(((raw + 1.0F) * float(max - min) / sineAmplitude) + min);
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

    [[nodiscard]] auto getFrequency() const -> float { return frequency; }

    constexpr void setFrequency(
      float newFrequency,
      std::optional<float> newSampleRate = std::nullopt)
    {
        if (newSampleRate.has_value()) {
            sampleRate = newSampleRate.value();
        }
        frequency = newFrequency;
        delta = float(wavetable.size()) * frequency / sampleRate;
    }

    /**
     * @brief
     *
     * @param out
     * @param phase
     * @todo what units should phase have??
     */
    void increment(T& out, float phase = 0.0F)
    {
        float phaseOffset = float(wavetable.size()) * phase / sampleRate;
        auto index0 = (unsigned int)(fractionalIndex + phase);
        if (index0 > wavetable.size()) {
            index0 -= wavetable.size();
        }
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

    void increment(std::span<T> out, float phase = 0.0F)
    {
        for (auto& sample : out) {
            increment(sample, phase);
        }
    }

private:
    static constexpr float defaultSampleRate = 96000.0F;
    std::span<const T> wavetable;
    float fractionalIndex{};
    float delta{};
    float frequency{};
    float sampleRate{ defaultSampleRate };
};

template<typename T>
WavetableOsc(const T*) -> WavetableOsc<T>;

template<typename T, size_t N>
WavetableOsc(std::array<T, N>) -> WavetableOsc<T>;