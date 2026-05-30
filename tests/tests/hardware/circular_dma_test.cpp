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
    CircularDma dma(memData, periphData);

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
    dma.execute<DmaDirection::MemoryToPeripheral>();
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
    dma.execute<DmaDirection::MemoryToPeripheral>();

    EXPECT_FALSE(dma.isReady());
    EXPECT_THAT(firstHalf, testing::Each(testing::Eq(newValue)));
    EXPECT_THAT(lastHalf, testing::Each(testing::Eq(oldValue)));

    fakeCompleteCallback();
    EXPECT_TRUE(dma.isReady());

    // (3) Do processing... copy data one last time
    oldValue = newValue;
    newValue = UINT16_MAX / 2;
    mock_operation<uint16_t>(memData, newValue);
    dma.execute<DmaDirection::MemoryToPeripheral>();
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

    // Initialize the peripheral buffer
    for (auto [idx, data] : std::views::enumerate(periphData)) {
        data = idx % nChannels;
    }

    // This is what the DMA manager should do
    for (auto i : std::views::iota(size_t(0), nChannels)) {
        auto channelBuf = memData | std::views::drop(i * channelBufSize) |
                          std::views::take(channelBufSize);
        auto deinterleavedPeriphBuf =
          periphData | std::views::drop(i) | std::views::stride(nChannels);
        auto zipped = std::views::zip(channelBuf, deinterleavedPeriphBuf);
        for (auto [out, in] : zipped) {
            out = in;
        }
    }

    for (auto i : std::views::iota(size_t(0), nChannels)) {
        EXPECT_THAT(memData | std::views::drop(i * channelBufSize) |
                      std::views::take(channelBufSize),
                    testing::Each(testing::Eq(i)));
    }
}

TEST(MultiChannelDma, Interleave) // MemoryToPeripheral
{
    EXPECT_TRUE(false);
}