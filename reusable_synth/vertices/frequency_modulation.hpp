/**
 * @file frequency_modulation.hpp
 * @author Chris DeFrancisci (chrisdefrancisci@gmail.com)
 * @brief
 * @date 2026-07-14
 */

#include <optional>

#include <reusable_synth/software/vertex_interface.hpp>
#include <reusable_synth/software/wavetable_osc.hpp>

/**
 * @briefd
 *
 * \f[
 * y(t) = A \space sin \bigg(\omega_c t + \frac{B}{\omega_m} sin(\omega_m
 * t)\bigg)
 * \f]
 * @tparam DataType
 * @tparam Size
 */
template<typename DataType, size_t Size>
class FrequencyModulation : public VertexInterface<DataType, Size>
{
public:
    using typename VertexInterface<DataType, Size>::ConnectFunc;

    /** Index in the inputs vector of parameter Amplitude, \f$ A \f$ */
    static constexpr size_t connectA = 0;
    /** Index in the inputs vector of parameter Modulation Index, \f$ B \f$ */
    static constexpr size_t connectB = 1;
    /** Index in the inputs vector of parameter Carrier Frequency, \f$ \omega_c
     * \f$ */
    static constexpr size_t connectFc = 2;
    /** Index in the inputs vector of parameter Modulation Frequency, \f$
     * \omega_m \f$ */
    static constexpr size_t connectFm = 3;

    FrequencyModulation(int id,
                        float sampleRate,
                        const std::span<const T> modWavetable,
                        const std::span<const T> carWavetable,
                        std::pmr::memory_resource* mem)
      : VertexInterface<DataType, Size>(id, mem)
      , sampleRate(sampleRate)
      , modOsc(modWavetable)
      , carrierOsc(carWavetable)
    {
    }

    FrequencyModulation(int id,
                        float sampleRate,
                        const std::span<const T> modWavetable,
                        std::pmr::memory_resource* mem)
      : FrequencyModulation(id, sampleRate, modWavetable, modWavetable, mem)
    {
    }

    [[nodiscard]] auto getInputs() const
      -> std::span<const ConnectFunc> override
    {
        return inputs;
    }

    void execute() override
    {
        if (modFreqBuff.empty() || carFreqBuff.empty()) {
            return;
        }

        for (size_t i = 0; i < Size; i++) {
            DataType amplitude =
              !amplitudeBuff.empty() ? amplitudeBuff[i] : DataType(1.0);
            DataType modIndex =
              !modIdxBuff.empty() ? modIdxBuff[i] : DataType(1.0);

            DataType carrierFreq = carFreqBuff[i];
            if (carrierFreq != carrierOsc.getFrequency()) {
                carrierOsc.setFrequency(carrierFreq, sampleRate);
            }
            DataType modFreq = modFreqBuff[i];
            if (modFreq != modOsc.getFrequency()) {
                modOsc.setFrequency(modFreq, sampleRate);
            }

            DataType phase = 0;
            modOsc.increment(phase);
            DataType output = 0;
            carrierOsc.increment(output, phase * modIndex / modFreq);

            this->getOutput(i) = output;
        }
    }

private:
    std::span<const DataType>
      amplitudeBuff{}; // Fixed size span does not have default ctor
    std::span<const DataType>
      modIdxBuff{}; // Fixed size span does not have default ctor
    std::span<const DataType>
      carFreqBuff{}; // Fixed size span does not have default ctor
    std::span<const DataType>
      modFreqBuff{}; // Fixed size span does not have default ctor

    void inputAmp(std::span<const DataType, Size> outputBuff)
    {
        static_cast<FrequencyModulation<DataType, Size>*>(this)->amplitudeBuff =
          outputBuff;
    }
    void inputModIdx(std::span<const DataType, Size> outputBuff)
    {
        static_cast<FrequencyModulation<DataType, Size>*>(this)->modIdxBuff =
          outputBuff;
    }
    void inputFc(std::span<const DataType, Size> outputBuff)
    {
        static_cast<FrequencyModulation<DataType, Size>*>(this)->carFreqBuff =
          outputBuff;
    }
    void inputFm(std::span<const DataType, Size> outputBuff)
    {
        static_cast<FrequencyModulation<DataType, Size>*>(this)->modFreqBuff =
          outputBuff;
    }

    std::array<ConnectFunc, 4> inputs{
        static_cast<ConnectFunc>(
          &FrequencyModulation<DataType, Size>::inputAmp),
        static_cast<ConnectFunc>(
          &FrequencyModulation<DataType, Size>::inputModIdx),
        static_cast<ConnectFunc>(&FrequencyModulation<DataType, Size>::inputFc),
        static_cast<ConnectFunc>(&FrequencyModulation<DataType, Size>::inputFm),

    };

    float sampleRate;
    WavetableOsc<DataType> modOsc;
    WavetableOsc<DataType> carrierOsc;
};