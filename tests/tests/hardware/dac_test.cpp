#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include <algorithm>
#include <array>
#include <ranges>

#include <reusable_synth/hardware/dac.hpp>
#include <reusable_synth/hardware/interrupt_handler.hpp>

constexpr float COMPUTATION_MIN = -1.0;
constexpr float COMPUTATION_MAX = 1.0;
constexpr int DAC_MIN = 0;
constexpr int DAC_MAX = 4096;

template<typename T>
static constexpr void mockOperation(std::span<T> out, T in)
{
    for (auto& item : out) {
        item = in;
    }
}

static constexpr void convert(uint16_t& out, const float& in)
{
    out = (uint16_t)std::min(
      std::max((in - COMPUTATION_MIN) / (COMPUTATION_MAX - COMPUTATION_MIN) *
                 (float)DAC_MAX,
               (float)DAC_MIN),
      (float)DAC_MAX);
}

TEST(DacTest, WriteToDoubleBuffer)
{
    constexpr int computationBufSize = 100;
    constexpr int DacBufSize = computationBufSize * 2;
    std::array<float, computationBufSize> computedData = { 0.0F };
    std::array<uint16_t, DacBufSize> dacData = { 0 };
    Dac dac(computedData, dacData, convert);

    InterruptHandler fakeHalfCompleteCallback;
    InterruptHandler fakeCompleteCallback;
    fakeHalfCompleteCallback.connect<&decltype(dac)::setHalfCompleteFlag>(&dac);
    fakeCompleteCallback.connect<&decltype(dac)::setCompleteFlag>(&dac);

    // Before callbacks, all dacData should be 0
    EXPECT_THAT(dacData, testing::Each(0));
    EXPECT_TRUE(dac.isReady());
    // Do processing... copy data
    mockOperation<float>(computedData, 1.0);
    dac.execute();
    EXPECT_FALSE(dac.isReady());
    // Initializes with FullComplete, first half should be 0 (uninitialized, not
    // max negative value), second half should be DAC_MAX
    auto firstHalf = dacData | std::views::take(dacData.size() / 2);
    auto lastHalf = dacData | std::views::drop(dacData.size() / 2);

    EXPECT_THAT(firstHalf, testing::Each(testing::Eq(0)));
    EXPECT_THAT(lastHalf, testing::Each(testing::Eq(DAC_MAX)));

    fakeHalfCompleteCallback();
    EXPECT_TRUE(dac.isReady());

    // Do processing... copy data again
    mockOperation<float>(computedData, 0.0);
    dac.execute();
    EXPECT_FALSE(dac.isReady());
    // After HalfComplete, first half should be DAC_MAX / 2, second half should
    // be DAC_MAX
    EXPECT_THAT(firstHalf, testing::Each(testing::Eq(DAC_MAX / 2)));
    EXPECT_THAT(lastHalf, testing::Each(testing::Eq(DAC_MAX)));

    fakeCompleteCallback();
    EXPECT_TRUE(dac.isReady());

    // Do processing... copy data one last time
    mockOperation<float>(computedData, -1.0);
    dac.execute();
    EXPECT_FALSE(dac.isReady());
    // After Complete, first half should be DAC_MAX / 2, second half should be 0
    // (now as an initialized, negative value)
    EXPECT_THAT(firstHalf, testing::Each(testing::Eq(DAC_MAX / 2)));
    EXPECT_THAT(lastHalf, testing::Each(testing::Eq(0)));

    // fakeHalfCompleteCallback();
}
