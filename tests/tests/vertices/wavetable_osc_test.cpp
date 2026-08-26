#include <array>
#include <cmath>
#include <numbers>

#include <gmock/gmock.h>
#include <gtest/gtest.h>
#include <tests/fixtures/graph_fixture.hpp>

#include <reusable_synth/software/wavetables.hpp>
#include <reusable_synth/vertices/wavetable_osc.hpp>

using namespace testing;
static constexpr size_t size = 9;
using WavetableOscTest = GraphFixture<float, size>;

// Just a note to self since I always forget:
// y[n] = sin(2 * pi * desired freq / sampling rate * table sampling rate /
//            table freq * n)
// = sin(2 *pi * N samples in table * desired freq / sampling rate * n)
// if there is 1 single period in the N sample table

TEST_F(WavetableOscTest, SquareWaveAtFsOver8)
{
    constexpr auto squareWavetable = SquareWavetable<float, 8>(-1.0F, 1.0F);

    // TODO: look at why implicit type deduction fails here
    std::array<float, 8> truth = { -1, -1, -1, -1, 1, 1, 1, 1 };

    EXPECT_THAT(squareWavetable.data, Pointwise(FloatEq(), truth));
}

TEST_F(WavetableOscTest, RampWave)
{
    constexpr auto rampWavetable = RampWavetable<float, 8>(-1.0F, 1.0F);

    std::array<float, 8> truth = { -1.0, -0.75, -0.5, -0.25,
                                   0.0,  0.25,  0.5,  0.75 };
    EXPECT_THAT(rampWavetable.data, Pointwise(FloatNear(0.001), truth));
}

TEST_F(WavetableOscTest, RampWaveInt)
{
    constexpr auto rampWavetable = RampWavetable<int, 8>(0, 4);

    std::array truth = { 0, 0, 1, 1, 2, 2, 3, 3 };

    EXPECT_THAT(rampWavetable.data, Pointwise(Eq(), truth));
}

TEST_F(WavetableOscTest, SineWaveAtFsOver8)
{
    constexpr int length = 8;
    SineWavetable<float, length> sineWavetable(-1, 1);

    std::array<float, length> truth{};
    for (int n = 0; auto& item : truth) {
        item =
          std::sin(std::numbers::pi_v<float> * 2.0F * float(n++) / (length));
    }

    EXPECT_THAT(sineWavetable.data, Pointwise(FloatNear(0.001), truth));
}

TEST_F(WavetableOscTest, SineWaveIntAtFsOver8)
{
    constexpr int length = 8;
    SineWavetable<int, length> sineWavetable(0, 255);

    std::array truth = { 127, 217, 255, 217, 127, 37, 0, 37 };

    EXPECT_THAT(sineWavetable.data, Pointwise(Eq(), truth));
}

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
    phase.fill(0.5F); // pi/2 mapped from (-1, +1) to (-pi, +pi)

    auto connectFrequency =
      graph[idx]->getInputs()[WavetableOsc<float, size>::connectFrequency];
    auto connectPhase =
      graph[idx]->getInputs()[WavetableOsc<float, size>::connectPhase];
    std::invoke(connectFrequency, graph[idx], freq);
    std::invoke(connectPhase, graph[idx], phase);

    executeSorted();

    EXPECT_THAT(graph[idx]->getOutput(), Pointwise(FloatNear(0.001), truth));
}

// TODO: not entirely sure I've proved out phase input sufficiently.
//  May want to add a test where I feed in a triangle waveform into the phase
//  input