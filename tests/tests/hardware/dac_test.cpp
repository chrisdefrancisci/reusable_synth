#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include <algorithm>
#include <array>
#include <ranges>

#include <reusable_synth/hardware/dac.hpp>
#include <reusable_synth/hardware/interrupt_handler.hpp>

constexpr int DAC_MAX = 4096;

static constexpr void convert(uint16_t& out, const float& in)
{
    out = std::min(std::max((in + 1.0f) / 2.0f * (float)DAC_MAX, 0.0f),
                   (float)DAC_MAX);
}

TEST(DacTest, WriteToDoubleBuffer)
{
    std::array<float, 100> computedData = { 0.0f };
    std::array<uint16_t, 200> dacData = { 0 };
    auto mockProcess = [&computedData](float value) -> void {
        for (auto& data : computedData) {
            data = value;
        }
    };
    Dac dac(computedData, dacData, convert);

    InterruptHandler fakeHalfCompleteCallback, fakeCompleteCallback;
    fakeHalfCompleteCallback
      .connect<&Dac<float, uint16_t, 100>::setHalfCompleteFlag>(&dac);
    fakeCompleteCallback.connect<&Dac<float, uint16_t, 100>::setCompleteFlag>(
      &dac);

    // Before callbacks, all dacData should be 0
    EXPECT_THAT(dacData, testing::Each(0));
    // Do processing... copy data
    mockProcess(1.0);
    dac.execute();
    // Initializes with FullComplete, first half should be 0 (uninitialized, not
    // max negative value), second half should be DAC_MAX
    auto firstHalf = dacData | std::views::take(dacData.size() / 2);
    auto lastHalf = dacData | std::views::drop(dacData.size() / 2);

    EXPECT_THAT(firstHalf, testing::Each(testing::Eq(0)));
    EXPECT_THAT(lastHalf, testing::Each(testing::Eq(DAC_MAX)));

    fakeHalfCompleteCallback();

    // Do processing... copy data again
    mockProcess(0.0);
    dac.execute();
    // After HalfComplete, first half should be DAC_MAX / 2, second half should
    // be DAC_MAX
    EXPECT_THAT(firstHalf, testing::Each(testing::Eq(DAC_MAX / 2)));
    EXPECT_THAT(lastHalf, testing::Each(testing::Eq(DAC_MAX)));

    fakeCompleteCallback();

    // Do processing... copy data one last time
    mockProcess(-1.0);
    dac.execute();
    // After Complete, first half should be DAC_MAX / 2, second half should be 0
    // (now as an initialized, negative value)
    EXPECT_THAT(firstHalf, testing::Each(testing::Eq(DAC_MAX / 2)));
    EXPECT_THAT(lastHalf, testing::Each(testing::Eq(0)));

    // fakeHalfCompleteCallback();
}
