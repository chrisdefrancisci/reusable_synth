#include <array>
#include <cmath>
#include <numbers>
#include <ranges>

#include <gmock/gmock.h>
#include <gtest/gtest.h>
#include <tests/fixtures/graph_fixture.hpp>

#include <reusable_synth/software/wavetables.hpp>
#include <reusable_synth/utils/conversions.hpp>
#include <reusable_synth/vertices/wavetable_osc.hpp>

using namespace testing;
static constexpr size_t size = 9;
using WavetableOscTest = GraphFixture<float, size>;

// Just a note to self since I always forget:
// y[n] = sin(2 * pi * desired freq / sampling rate * table sampling rate /
//            table freq * n)
// = sin(2 *pi * N samples in table * desired freq / sampling rate * n)
// if there is 1 single period in the N sample table

TEST_F(WavetableOscTest, CosineWaveAtFsOver8)
{
    constexpr float sampleRate = 44100;
    constexpr float oscFreq = sampleRate / 8;
    constexpr int length = 32;
    SineWavetable<float, length> sineWavetable(-1, 1);
    auto idx = graph.addVertex<WavetableOsc<float, size>>(sampleRate,
                                                          sineWavetable.data);
    constexpr int nSamples = 8; // number of samples in a period
    std::array<float, size> truth{};
    for (int n = 0; auto& item : truth) {
        item =
          std::cos(2.0F * std::numbers::pi_v<float> * float(n++) / (nSamples));
    }

    std::array<float, size> freq{};
    freq.fill(oscFreq);
    std::array<float, size> phase{};
    phase.fill(Conversions::phase_to_cv(std::numbers::pi_v<float> / 2.0F));

    auto connectFrequency =
      graph[idx]->getInputs()[WavetableOsc<float, size>::connectFrequency];
    auto connectPhase =
      graph[idx]->getInputs()[WavetableOsc<float, size>::connectPhase];
    std::invoke(connectFrequency, graph[idx], freq);
    std::invoke(connectPhase, graph[idx], phase);

    executeSorted();

    EXPECT_THAT(graph[idx]->getOutput(), Pointwise(FloatNear(0.001), truth));
}

TEST_F(WavetableOscTest, PhaseChangeAtFsOver8)
{
    // Set up the oscillator
    constexpr float sampleRate = 96000.0F;
    constexpr float oscFreq = sampleRate / 8;
    constexpr int length = 64;
    SineWavetable<float, length> sineWavetable(-1, 1);
    auto idx = graph.addVertex<WavetableOsc<float, size>>(sampleRate,
                                                          sineWavetable.data);
    // Set up the true value
    constexpr size_t firstPhaseShift = 14;  // Arbitrary
    constexpr size_t secondPhaseShift = 20; // Arbitrary, but want to be in the
                                            // middle of execution loop
    float truePhase = 0.0;
    constexpr int nExecute = 3;
    std::array<float, size * nExecute> truth{};
    for (int n = 0; auto& item : truth) {
        if (firstPhaseShift <= n && n < secondPhaseShift) {
            truePhase = std::numbers::pi_v<float>;
        } else if (n >= secondPhaseShift) {
            truePhase = -std::numbers::pi_v<float>;
        }
        item = std::sin((2.0F * std::numbers::pi_v<float> * float(n++) *
                         oscFreq / sampleRate) +
                        truePhase);
    }

    // Set up the frequency, phase input, phase will be 0, +pi/2, -pi/2
    std::array<float, size> freq{};
    freq.fill(oscFreq);
    std::array<float, size * nExecute> phase{};
    for (auto& sample :
         phase | std::views::drop(firstPhaseShift) |
           std::views::take(secondPhaseShift - firstPhaseShift)) {
        sample = 1.0F;
    }
    for (auto& sample : phase | std::views::drop(secondPhaseShift) |
                          std::views::take(phase.size() - secondPhaseShift)) {
        sample = -1.0F;
    }

    // Connect inputs to oscillator
    std::span<const float, size> phaseSubset(phase | std::views::take(size));
    auto connectFrequency =
      graph[idx]->getInputs()[WavetableOsc<float, size>::connectFrequency];
    auto connectPhase =
      graph[idx]->getInputs()[WavetableOsc<float, size>::connectPhase];
    std::invoke(connectFrequency, graph[idx], freq);
    std::invoke(connectPhase, graph[idx], phaseSubset);

    // Run the test
    for (int i = 0; i < nExecute; i++) {
        phaseSubset = std::span<const float, size>(
          phase | std::views::drop(i * size) | std::views::take(size));
        // TODO: this is silly to reconnect over and over again
        // But moving the span after it is assigned doesn't actually do
        // anything for the wave osc input variable
        std::invoke(connectPhase, graph[idx], phaseSubset);

        executeSorted();

        auto truthSubset = std::span<float, size>(
          truth | std::views::drop(i * size) | std::views::take(size));
        EXPECT_THAT(graph[idx]->getOutput(),
                    Pointwise(FloatNear(0.001), truthSubset))
          << "Failure at iteration: " << i;
    }
}
