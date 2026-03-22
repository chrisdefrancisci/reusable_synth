#include <array>
#include <cmath>
#include <numbers>

#include <gmock/gmock.h>
#include <gtest/gtest.h>
#include <reusable_synth/software/wavetable_osc.hpp>

using namespace testing;

TEST(WavetableOscTest, SquareWaveAtFsOver8)
{
    constexpr float f_s = 44100;
    constexpr float f_osc = f_s / 8;
    constexpr auto squareWavetable = SquareWavetable<float, 32>();

    // TODO: look at why implicit type deduction fails here
    WavetableOsc<float> osc(squareWavetable.data);
    std::array<float, 9> truth = { -1, -1, -1, -1, 1, 1, 1, 1, -1 };
    std::array<float, 9> out;
    osc.setFrequency(f_osc, f_s);
    osc.increment(out);

    EXPECT_THAT(out, Pointwise(FloatEq(), truth));
}

TEST(WavetableOscTest, RampWaveAtFsOver8)
{
    constexpr float f_s = 44100;
    constexpr float f_osc = f_s / 8;
    constexpr auto rampWavetable = RampWavetable<float, 32>();

    WavetableOsc<float> osc(rampWavetable.data);
    std::array<float, 9> truth = { -1.0, -0.75, -0.5, -0.25, 0.0,
                                   0.25, 0.5,   0.75, -1.0 };
    std::array<float, 9> out;
    osc.setFrequency(f_osc, f_s);
    osc.increment(out);

    EXPECT_THAT(out, Pointwise(FloatNear(0.001), truth));
}

TEST(WavetableOscTest, RampWaveIntAtFsOver8)
{
    constexpr float f_s = 44100;
    constexpr float f_osc = f_s / 8;
    constexpr auto rampWavetable = RampWavetable<int, 8, 0, 4>();

    WavetableOsc osc(rampWavetable.data);
    std::array truth = { 0, 0, 1, 1, 2, 2, 3, 3, 0 };
    decltype(truth) out;
    osc.setFrequency(f_osc, f_s);
    osc.increment(out);

    EXPECT_THAT(out, Pointwise(Eq(), truth));
}

TEST(WavetableOscTest, SineWaveAtFsOver8)
{
    constexpr float f_s = 44100;
    constexpr float f_osc = f_s / 8;
    constexpr int N = 32;
    std::array<float, N> data;
    for (int n = 0; auto& item : data) {
        item = std::sin(std::numbers::pi_v<float> * 2.0 * n++ / N);
    }
    WavetableOsc<float> osc(data);

    constexpr int N_true = 8;
    std::array<float, 9> truth;
    for (int n = 0; auto& item : truth) {
        item = std::sin(std::numbers::pi_v<float> * 2.0 * n++ / (N_true));
    }

    std::array<float, 9> out;
    osc.setFrequency(f_osc, f_s);
    osc.increment(out);

    EXPECT_THAT(out, Pointwise(FloatNear(0.001), truth));
}

TEST(WavetableOscTest, SineWaveUndersampled)
{
    EXPECT_TRUE(false);
}

TEST(WavetableOscTest, SineWaveOversampled)
{
    EXPECT_TRUE(false);
}

TEST(WavetableOscTest, SineWaveAtMultipleFrequencies)
{
    EXPECT_TRUE(false);
}