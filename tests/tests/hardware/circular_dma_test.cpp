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

TEST(CircularDma, WriteToDoubleBuffer)
{
    constexpr int memBufSize = 100;
    constexpr int periphBufSize = memBufSize * 2;
    // Data manipulated in memory
    std::array<uint16_t, memBufSize> memData = { 0 };
    // Data for the peripheral
    std::array<uint16_t, periphBufSize> periphData = { 0 };
    auto dma = make_circulardma<DmaDirection::MemoryToPeripheral>(
      std::span(memData), std::span(periphData));

    InterruptHandler fakeHalfCompleteCallback;
    InterruptHandler fakeCompleteCallback;
    fakeHalfCompleteCallback.connect<&decltype(dma)::setHalfCompleteFlag>(&dma);
    fakeCompleteCallback.connect<&decltype(dma)::setCompleteFlag>(&dma);

    // Before callbacks, all dacData should be 0
    uint16_t oldValue = 0;
    EXPECT_THAT(periphData, testing::Each(oldValue));
    EXPECT_TRUE(dma.isReady());
    // (1) Do processing... copy data
    uint16_t newValue = 1;
    mock_operation<uint16_t>(memData, newValue);
    dma.execute();
    EXPECT_FALSE(dma.isReady());
    auto firstHalf = periphData | std::views::take(periphData.size() / 2);
    auto lastHalf = periphData | std::views::drop(periphData.size() / 2);

    EXPECT_THAT(firstHalf, testing::Each(testing::Eq(oldValue)));
    EXPECT_THAT(lastHalf, testing::Each(testing::Eq(newValue)));

    fakeHalfCompleteCallback();
    EXPECT_TRUE(dma.isReady());

    // (2) Do processing... copy data again
    oldValue = newValue;
    newValue = UINT16_MAX;
    mock_operation<uint16_t>(memData, newValue);
    dma.execute();

    EXPECT_FALSE(dma.isReady());
    EXPECT_THAT(firstHalf, testing::Each(testing::Eq(newValue)));
    EXPECT_THAT(lastHalf, testing::Each(testing::Eq(oldValue)));

    fakeCompleteCallback();
    EXPECT_TRUE(dma.isReady());

    // (3) Do processing... copy data one last time
    oldValue = newValue;
    newValue = UINT16_MAX / 2;
    mock_operation<uint16_t>(memData, newValue);
    dma.execute();
    EXPECT_FALSE(dma.isReady());
    EXPECT_THAT(firstHalf, testing::Each(testing::Eq(oldValue)));
    EXPECT_THAT(lastHalf, testing::Each(testing::Eq(newValue)));

    // fakeHalfCompleteCallback();
}

TEST(MultiChannelDma, Deinterleave) // PeripheralToMemory
{
    constexpr size_t nChannels = 10;
    constexpr size_t channelBufSize = 100;
    constexpr size_t memBufSize = nChannels * channelBufSize;
    constexpr size_t periphBufSize = memBufSize * 2;
    std::array<uint16_t, memBufSize> memData{};
    std::array<uint16_t, periphBufSize> periphData{};
    auto dma = make_circulardma<DmaDirection::PeripheralToMemory, nChannels>(
      std::span(memData), std::span(periphData));

    // Initialize the peripheral buffer
    // Delineate between channel and front/back half
    for (auto [idx, data] : std::views::enumerate(periphData)) {
        if (idx < long(periphData.size()) / 2) {
            data = idx % nChannels;
        } else {
            data = (idx % nChannels) + 1;
        }
    }

    InterruptHandler fakeHalfCompleteCallback;
    InterruptHandler fakeCompleteCallback;
    fakeHalfCompleteCallback.connect<&decltype(dma)::setHalfCompleteFlag>(&dma);
    fakeCompleteCallback.connect<&decltype(dma)::setCompleteFlag>(&dma);

    // Copy first half of peripheral buffer to memory
    EXPECT_FALSE(dma.isReady());
    fakeHalfCompleteCallback();
    EXPECT_TRUE(dma.isReady());
    dma.execute();
    for (auto i : std::views::iota(size_t(0), nChannels)) {
        EXPECT_THAT(memData | std::views::drop(i * channelBufSize) |
                      std::views::take(channelBufSize),
                    testing::Each(testing::Eq(i)));
    }

    // Copy second half of peripheral buffer to memory
    // Note that now the value is increased by one
    EXPECT_FALSE(dma.isReady());
    fakeCompleteCallback();
    EXPECT_TRUE(dma.isReady());
    dma.execute();
    for (auto i : std::views::iota(size_t(0), nChannels)) {
        EXPECT_THAT(memData | std::views::drop(i * channelBufSize) |
                      std::views::take(channelBufSize),
                    testing::Each(testing::Eq(i + 1)));
    }
}

TEST(MultiChannelDma, Interleave) // MemoryToPeripheral
{
    constexpr size_t nChannels = 10;
    constexpr size_t channelBufSize = 100;
    constexpr size_t memBufSize = nChannels * channelBufSize;
    constexpr size_t periphBufSize = memBufSize * 2;
    std::array<uint16_t, memBufSize> memData{};
    std::array<uint16_t, periphBufSize> periphData{};
    auto dma = make_circulardma<DmaDirection::MemoryToPeripheral, nChannels>(
      std::span(memData), std::span(periphData));

    InterruptHandler fakeHalfCompleteCallback;
    InterruptHandler fakeCompleteCallback;
    fakeHalfCompleteCallback.connect<&decltype(dma)::setHalfCompleteFlag>(&dma);
    fakeCompleteCallback.connect<&decltype(dma)::setCompleteFlag>(&dma);

    // Initialize the memory buffer
    for (auto [idx, channel] :
         std::views::enumerate(memData | std::views::chunk(channelBufSize))) {
        for (auto& val : channel) {
            val = (idx + 1) * 2;
        }
    }

    // Initially assume DMA peripheral is operating on first half,
    // copy memory to second half of peripheral buffer
    EXPECT_TRUE(dma.isReady());
    dma.execute();
    EXPECT_FALSE(dma.isReady());

    // Check second half of peripheral buffer
    for (auto [idx, val] :
         std::views::enumerate(periphData | std::views::drop(memBufSize))) {
        EXPECT_EQ(val, (idx % nChannels + 1) * 2);
    }

    // Initialize the memory buffer for a second time with some other set of
    // values
    for (auto [idx, channel] :
         std::views::enumerate(memData | std::views::chunk(channelBufSize))) {
        for (auto& val : channel) {
            val = (idx + 1);
        }
    }

    // Copy memory to first half of peripheral buffer
    fakeHalfCompleteCallback();
    EXPECT_TRUE(dma.isReady());
    dma.execute();
    EXPECT_FALSE(dma.isReady());

    // Check first half of buffer
    for (auto [idx, val] :
         std::views::enumerate(periphData | std::views::take(memBufSize))) {
        EXPECT_EQ(val, (idx % nChannels) + 1);
    }

    // Initialize the memory buffer for a third time with some other set of
    // values
    for (auto [idx, channel] :
         std::views::enumerate(memData | std::views::chunk(channelBufSize))) {
        for (auto& val : channel) {
            val = (idx + 5);
        }
    }

    // Copy memory to second half of peripheral buffer
    fakeCompleteCallback();
    EXPECT_TRUE(dma.isReady());
    dma.execute();
    EXPECT_FALSE(dma.isReady());

    // Check second half of buffer
    for (auto [idx, val] :
         std::views::enumerate(periphData | std::views::drop(memBufSize))) {
        EXPECT_EQ(val, (idx % nChannels) + 5);
    }
}