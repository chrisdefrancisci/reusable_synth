#include <cstdint>
#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include <algorithm>
#include <array>
#include <ranges>

#include <reusable_synth/hardware/circular_dma.hpp>
#include <reusable_synth/hardware/interrupt_handler.hpp>

template<typename T>
static constexpr void mock_operation(std::span<T> out, T in)
{
    for (auto& item : out) {
        item = in;
    }
}

TEST(DacTest, WriteToDoubleBuffer)
{
    constexpr int computationBufSize = 100;
    constexpr int dacBufSize = computationBufSize * 2;
    std::array<uint16_t, computationBufSize> computedData = { 0 };
    std::array<uint16_t, dacBufSize> dacData = { 0 };
    CircularDma dac(computedData, dacData);

    InterruptHandler fakeHalfCompleteCallback;
    InterruptHandler fakeCompleteCallback;
    fakeHalfCompleteCallback.connect<&decltype(dac)::setHalfCompleteFlag>(&dac);
    fakeCompleteCallback.connect<&decltype(dac)::setCompleteFlag>(&dac);

    // Before callbacks, all dacData should be 0
    uint16_t oldValue = 0;
    EXPECT_THAT(dacData, testing::Each(oldValue));
    EXPECT_TRUE(dac.isReady());
    // (1) Do processing... copy data
    uint16_t newValue = 1;
    mock_operation<uint16_t>(computedData, newValue);
    dac.execute<DmaDirection::MemoryToPeripheral>();
    EXPECT_FALSE(dac.isReady());
    auto firstHalf = dacData | std::views::take(dacData.size() / 2);
    auto lastHalf = dacData | std::views::drop(dacData.size() / 2);

    EXPECT_THAT(firstHalf, testing::Each(testing::Eq(oldValue)));
    EXPECT_THAT(lastHalf, testing::Each(testing::Eq(newValue)));

    fakeHalfCompleteCallback();
    EXPECT_TRUE(dac.isReady());

    // (2) Do processing... copy data again
    oldValue = newValue;
    newValue = UINT16_MAX;
    mock_operation<uint16_t>(computedData, newValue);
    dac.execute<DmaDirection::MemoryToPeripheral>();

    EXPECT_FALSE(dac.isReady());
    EXPECT_THAT(firstHalf, testing::Each(testing::Eq(newValue)));
    EXPECT_THAT(lastHalf, testing::Each(testing::Eq(oldValue)));

    fakeCompleteCallback();
    EXPECT_TRUE(dac.isReady());

    // (3) Do processing... copy data one last time
    oldValue = newValue;
    newValue = UINT16_MAX / 2;
    mock_operation<uint16_t>(computedData, newValue);
    dac.execute<DmaDirection::MemoryToPeripheral>();
    EXPECT_FALSE(dac.isReady());
    EXPECT_THAT(firstHalf, testing::Each(testing::Eq(oldValue)));
    EXPECT_THAT(lastHalf, testing::Each(testing::Eq(newValue)));

    // fakeHalfCompleteCallback();
}
