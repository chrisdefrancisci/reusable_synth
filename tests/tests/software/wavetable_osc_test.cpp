#include "gmock/gmock.h"
#include <array>

#include <gmock/gmock.h>
#include <gtest/gtest.h>
#include <reusable_synth/software/wavetable_osc.hpp>

using namespace testing;

TEST(WavetableOscTest, SquareWaveAtNyquistOver8)
{
    constexpr float f_s = 44100;
    constexpr float f_osc = f_s / 8;
    constexpr auto squareWavetable = SquareWavetable<float, 2048>();

    // TODO: look at why implicit type deduction fails here
    WavetableOsc<float> osc(squareWavetable.data);
    std::array<float, 8> truth = { -1, -1, -1, -1, 1, 1, 1, 1 };
    std::array<float, 8> out;
    osc.setFrequency(f_osc, f_s);
    osc.increment(out);

    EXPECT_THAT(truth, Pointwise(FloatEq(), out));
}

TEST(WavetableOscTest, RampWaveAtNyquistOver8)
{
    EXPECT_TRUE(false);
}

TEST(WavetableOscTest, SineWaveAtNyquistOver8)
{
    EXPECT_TRUE(false);
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