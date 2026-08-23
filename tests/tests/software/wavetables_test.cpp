#include <array>
#include <cmath>
#include <numbers>

#include <gmock/gmock.h>
#include <gtest/gtest.h>
#include <reusable_synth/software/wavetables.hpp>

using namespace testing;

TEST(WavetableOscTest, SquareWaveAtFsOver8)
{
    constexpr auto squareWavetable = SquareWavetable<float, 8>(-1.0F, 1.0F);

    // TODO: look at why implicit type deduction fails here
    std::array<float, 8> truth = { -1, -1, -1, -1, 1, 1, 1, 1 };

    EXPECT_THAT(squareWavetable.data, Pointwise(FloatEq(), truth));
}

TEST(WavetableOscTest, RampWave)
{
    constexpr auto rampWavetable = RampWavetable<float, 8>(-1.0F, 1.0F);

    std::array<float, 8> truth = { -1.0, -0.75, -0.5, -0.25,
                                   0.0,  0.25,  0.5,  0.75 };
    EXPECT_THAT(rampWavetable.data, Pointwise(FloatNear(0.001), truth));
}

TEST(WavetableOscTest, RampWaveInt)
{
    constexpr auto rampWavetable = RampWavetable<int, 8>(0, 4);

    std::array truth = { 0, 0, 1, 1, 2, 2, 3, 3 };

    EXPECT_THAT(rampWavetable.data, Pointwise(Eq(), truth));
}

TEST(WavetableOscTest, SineWaveAtFsOver8)
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

TEST(WavetableOscTest, SineWaveIntAtFsOver8)
{
    constexpr int length = 8;
    SineWavetable<int, length> sineWavetable(0, 255);

    std::array truth = { 127, 217, 255, 217, 127, 37, 0, 37 };

    EXPECT_THAT(sineWavetable.data, Pointwise(Eq(), truth));
}

// TEST(WavetableOscTest, CosineWaveAtFsOver8)
// {
//     constexpr float samplingFreq = 44100;
//     constexpr float oscFreq = samplingFreq / 8;
//     constexpr int length = 32;
//     constexpr float phase = std::numbers::pi_v<float> / 2.0F;
//     SineWavetable<float, length> sineWavetable(-1, 1);
//     WavetableOsc osc(sineWavetable.data);

//     constexpr int nSamples = 8; // number of samples in a period
//     std::array<float, 9> truth{};
//     for (int n = 0; auto& item : truth) {
//         item =
//           std::cos(2.0F * std::numbers::pi_v<float> * float(n++) /
//           (nSamples));
//     }

//     decltype(truth) out;
//     osc.setFrequency(oscFreq, samplingFreq);
//     osc.increment(out, phase);

//     EXPECT_THAT(out, Pointwise(FloatNear(0.001), truth));
// }

// TODO: not entirely sure I've proved out phase input sufficiently.
//  May want to add a test where I feed in a triangle waveform into the phase
//  input