/**
 * @file wavetable_osc.hpp
 * @author Chris DeFrancisci (chrisdefrancisci@gmail.com)
 * @brief
 * @date 2026-08-16
 */

#include <algorithm>
#include <cmath>
#include <ranges>

#include <reusable_synth/software/vertex_interface.hpp>
#include <reusable_synth/software/wavetables.hpp>

// TODO: scaling from -1 to +1 -> frequency. Linear? Logarathmic?

// TODO: if first sample == last sample, we can optimize checking the indices
// see https://juce.com/tutorials/tutorial_wavetable_synth/
template<typename DataType, size_t Size>
class WavetableOsc : public VertexInterface<DataType, Size>
{
private:
    static constexpr float twopi = 2.0F * std::numbers::pi_v<float>;
    static constexpr float minFreqChange = 2.0F / 127.0F * 0.03;
    static constexpr auto convertPhase(DataType in) -> DataType
    {
        DataType minIn = -1;
        DataType maxIn = 1;
        DataType rangeIn = maxIn - minIn;
        in = std::clamp(in, minIn, maxIn);
        DataType minOut = -std::numbers::pi_v<DataType>;
        DataType maxOut = -minOut;
        DataType rangeOut = maxOut - minOut;
        return (rangeOut / rangeIn * (in - minIn)) + minOut;
    }
    static constexpr auto convertFreq(DataType in) -> DataType
    {
        DataType minIn = -1;
        DataType maxIn = 1;
        DataType rangeIn = maxIn - minIn;
        in = std::clamp(in, minIn, maxIn);
        DataType minOut = 0;
        constexpr DataType maxOut = 127;
        DataType rangeOut = maxOut - minOut;
        DataType midiNoteNumber = (rangeOut / rangeIn * (in - minIn)) + minOut;
        // From
        // https://en.wikipedia.org/wiki/Musical_note#MIDI
        // NOLINTNEXTLINE(*-magic-numbers)
        return std::pow(2, (midiNoteNumber - 69) / 12) * 440;
    }

public:
    using typename VertexInterface<DataType, Size>::ConnectFunc;

    static constexpr size_t connectFrequency = 0;
    static constexpr size_t connectPhase = 1;

    explicit WavetableOsc(int id,
                          const std::span<const DataType> wavetable,
                          std::pmr::memory_resource* mem)
      : VertexInterface<DataType, Size>(id, mem)
      , wavetable(wavetable)
    {
    }

    [[nodiscard]] auto getInputs() const
      -> std::span<const ConnectFunc> override
    {
        return inputs;
    }

    /**
     * @brief
     *
     */
    void execute() override
    {
        if (inputFreqBuff.size() == 0) {
            return;
        }

        auto out = this->getOutput();
        if (inputPhaseBuff.empty()) {
            for (auto& [sample, freq] : std::views::zip(out, frequency)) {
                setFrequency(convertFreq(freq));
                increment(sample);
            }
        } else {
            for (auto& [sample, freq, phase] :
                 std::views::zip(out, inputFreqBuff, inputPhaseBuff)) {
                setFrequency(convertFreq(freq));
                increment(sample, convertPhase(phase));
            }
        }
    }

    constexpr void setSampleRate(float newSampleRate)
    {
        sampleRate = newSampleRate;
        delta = float(wavetable.size()) * frequency / sampleRate;
    }

private:
    /**
     * @brief Wavetable increment calculation for a single sample
     *
     * @param out
     * @param phase in radians
     */
    void increment(DataType& out, float phase = 0.0F)
    {
        // Convert phase in radians to sampled table
        phase = phase < 0 ? phase + twopi : phase;
        float phaseOffset = float(wavetable.size()) * phase / twopi;
        auto index0 = (unsigned int)(fractionalIndex + phaseOffset);
        if (index0 >= wavetable.size()) {
            index0 -= wavetable.size();
        }
        auto index1 =
          index0 + 1 == wavetable.size() ? (unsigned int)0 : index0 + 1;
        float frac = fractionalIndex + phaseOffset - float(index0);
        if (frac >= wavetable.size()) {
            frac -= wavetable.size();
        }
        out = wavetable[index0] +
              (frac * (float(wavetable[index1]) - float(wavetable[index0])));
        fractionalIndex += delta;
        if (fractionalIndex >= wavetable.size()) {
            fractionalIndex -= wavetable.size();
        }
    }

    // TODO get rid of below
    constexpr void setFrequency(float newFrequency)
    {
        frequency = newFrequency;
        delta = float(wavetable.size()) * frequency / sampleRate;
    }
    // TODO get rid of above

    std::span<const DataType> inputFreqBuff{};
    std::span<const DataType> inputPhaseBuff{};

    void inputFreq(std::span<const DataType, Size> outputBuff)
    {
        static_cast<WavetableOsc<DataType, Size>*>(this)->inputFreqBuff =
          outputBuff;
    }

    void inputPhase(std::span<const DataType, Size> outputBuff)
    {
        static_cast<WavetableOsc<DataType, Size>*>(this)->inputPhaseBuff =
          outputBuff;
    }

    std::array<ConnectFunc, 2> inputs{
        static_cast<ConnectFunc>(&WavetableOsc<DataType, Size>::inputFreqBuff),
        static_cast<ConnectFunc>(&WavetableOsc<DataType, Size>::inputPhaseBuff)
    };

    static constexpr float defaultSampleRate = 96000.0F;
    std::span<const DataType> wavetable;
    float fractionalIndex{};
    float delta{};
    float frequency{};
    float sampleRate{ defaultSampleRate };
};