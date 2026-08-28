#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include <reusable_synth/utils/conversions.hpp>

using namespace Conversions;

TEST(Conversions, MapRange)
{
    // Note that because of integer division, the input range always needs to be
    // smaller if in
    EXPECT_EQ(int(2), map_range(1, { -1, 1 }, { -2, 2 }));
    EXPECT_FLOAT_EQ(1.0F, map_range(2.0F, { -2.0F, 2.0F }, { -1.0F, 1.0F }));
    EXPECT_FLOAT_EQ(0.0F, map_range(0.0F, { -2.0F, 2.0F }, { -1.0F, 1.0F }));
    // Test clamping
    EXPECT_FLOAT_EQ(1.0F, map_range(3.0F, { -2.0F, 2.0F }, { -1.0F, 1.0F }));
}

TEST(Conversions, CvToPhase)
{
    EXPECT_FLOAT_EQ(-std::numbers::pi_v<float>, cv_to_phase(-1.0F));
    EXPECT_FLOAT_EQ(-std::numbers::pi_v<float> / 2.0F, cv_to_phase(-0.5F));
    EXPECT_FLOAT_EQ(0.0F, cv_to_phase(0.0F));
    EXPECT_FLOAT_EQ(std::numbers::pi_v<float> / 2.0F, cv_to_phase(0.5F));
    EXPECT_FLOAT_EQ(std::numbers::pi_v<float>, cv_to_phase(1.0F));
}

TEST(Conversions, PhaseToCv)
{
    EXPECT_FLOAT_EQ(-1.0F, phase_to_cv(-std::numbers::pi_v<float>));
    EXPECT_FLOAT_EQ(-0.5F, phase_to_cv(-std::numbers::pi_v<float> / 2.0F));
    EXPECT_FLOAT_EQ(-0.0F, phase_to_cv(0.0F));
    EXPECT_FLOAT_EQ(0.5F, phase_to_cv(std::numbers::pi_v<float> / 2.0F));
    EXPECT_FLOAT_EQ(1.0F, phase_to_cv(std::numbers::pi_v<float>));
}

TEST(Conversions, CvToFrequency)
{
    EXPECT_NEAR(440.0F, cv_to_freq(0.086614F), 0.0005F);
    EXPECT_NEAR(8.176F, cv_to_freq(-1.0F), 0.0005F);
    EXPECT_NEAR(12543.85F, cv_to_freq(1.0F), 0.006F);
}

TEST(Conversions, FrequencyToCv)
{
    EXPECT_NEAR(440.0F, cv_to_freq(0.086614F), 0.0005F);
    EXPECT_NEAR(8.176F, cv_to_freq(-1.0F), 0.0005F);
    EXPECT_NEAR(12543.85F, cv_to_freq(1.0F), 0.006F);

    EXPECT_NEAR(0.086614F, freq_to_cv(440.0F), 0.0005F);
    EXPECT_NEAR(-1.0F, freq_to_cv(8.176F), 0.0005F);
    EXPECT_NEAR(1.0F, freq_to_cv(12543.85F), 0.005F);
}
