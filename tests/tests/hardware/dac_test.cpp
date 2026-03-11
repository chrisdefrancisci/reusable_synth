#include "gmock/gmock.h"
#include "gtest/gtest.h"
#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include <algorithm>
#include <array>

#include <reusable_synth/hardware/dac.hpp>
#include <reusable_synth/hardware/interrupt_handler.hpp>

constexpr int DAC_MAX = 4096;

static constexpr void convert(uint16_t& out, const float& in)
{
    out = std::min(std::max((in + 1.0f) / 2.0f * (float)DAC_MAX, 0.0f),
                   (float)DAC_MAX);
}

TEST(DacTest, OutputSizeMustBeDoubleInputSize)
{
    // Will not compile if the dac data is not 2x computed data
    std::array<float, 100> computedData;
    std::array<uint16_t, 200> dacData;
    // Note that template argument deduction cannot happen if implicit
    // type conversion needs to occur first - i.e., in this case we must convert
    // to span and then template argument deduction can hapen
    Dac dac(std::span(computedData), std::span(dacData), convert);
}

// TEST(DacTest, WriteToDac)
// {
// For this test I would need a mock DMA. don't want that.
// }

TEST(DacTest, ChangeDacPtr)
{
    std::array<float, 100> computedData = { 0.0f };
    std::array<uint16_t, 200> dacData = { 0 };
    auto mockProcess = [&computedData](float value) -> void {
        for (auto& data : computedData) {
            data = value;
        }
    };
    Dac dac(std::span(computedData), std::span(dacData), convert);

    InterruptHandler halfCompleteCallback, completeCallback;
    halfCompleteCallback
      .connect<&Dac<float, uint16_t, 100>::setHalfCompleteFlag>(&dac);
    completeCallback.connect<&Dac<float, uint16_t, 100>::setCompleteFlag>(&dac);

    // Before callbacks, all dacData should be 0
    EXPECT_THAT(dacData, testing::Contains(testing::FloatEq(0.0)).Times(200));
    // Do processing... copy data
    mockProcess(1.0);
    dac.execute();
    // After HalfComplete, first half should be 0 (uninitialized), second half
    // should be DAC_MAX
    halfCompleteCallback();
    EXPECT_THAT(std::span(dacData).subspan(100),
                testing::Contains(testing::Eq(DAC_MAX / 2)).Times(100));
    EXPECT_THAT(std::span(dacData).subspan(100, 100),
                testing::Contains(testing::Eq(DAC_MAX)).Times(100));

    // Do processing... copy data again
    mockProcess(-1.0);
    dac.execute();
    // After Complete, first half should be 0 , second half should be DAC_MAX
    completeCallback();
    EXPECT_THAT(std::span(dacData).subspan(0, 100),
                testing::Contains(testing::Eq(0)).Times(100));
    EXPECT_THAT(std::span(dacData).subspan(100, 100),
                testing::Contains(testing::Eq(DAC_MAX)).Times(100));

    // Do processing... copy data one last time
    mockProcess(0.0);
    dac.execute();
    // After Complete, first half should be 0 , second half should be DAC_MAX
    halfCompleteCallback();
    EXPECT_THAT(std::span(dacData).subspan(0, 100),
                testing::Contains(testing::Eq(DAC_MAX / 2)).Times(100));
    EXPECT_THAT(std::span(dacData).subspan(100, 100),
                testing::Contains(testing::Eq(DAC_MAX)).Times(100));
}

TEST(DacTest, ConvertTypes) {}
TEST(DacTest, BoundCheck) {}
TEST(DacTest, ConvertTypesAndBoundCheck) {}