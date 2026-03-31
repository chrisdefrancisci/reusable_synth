#include <gtest/gtest.h>

#include <reusable_synth/utils/ring_buffer.hpp>

TEST(RingBufferTest, EmptyBeforePush)
{
    constexpr size_t size = 10;
    RingBuffer<int, size> buff;
    auto val = buff.popFront();
    EXPECT_FALSE(val.has_value());
}

TEST(RingBufferTest, SinglePushPop)
{
    constexpr size_t size = 10;
    RingBuffer<int, size> buff;
    int testVal = 1;
    buff.pushBack(testVal);
    EXPECT_FALSE(buff.empty());
    auto val = buff.popFront();
    ASSERT_TRUE(val.has_value());
    EXPECT_EQ(val.value(), testVal);
}

TEST(RingBufferTest, PushFillPopAll)
{
    constexpr size_t size = 10;
    RingBuffer<int, size> buff;
    int i = 0;
    while (!buff.full()) {
        ASSERT_LT(i, size);
        buff.pushBack(i);
        i++;
    }
    EXPECT_EQ(i, size);

    i = 0;
    while (!buff.empty()) {
        auto val = buff.popFront();
        ASSERT_TRUE(val.has_value());
        EXPECT_EQ(val.value(), i);
        ASSERT_LT(i, size);
        i++;
    }
    EXPECT_FALSE(buff.popFront().has_value());
}

TEST(RingBufferTest, UnevenPushPop)
{
    constexpr size_t size = 10;
    RingBuffer<int, size> buff;
    int pushVal = 0;
    int expectedVal = 0;
    buff.pushBack(pushVal++);      // 0
    buff.pushBack(pushVal++);      // 1
    buff.pushBack(pushVal++);      // 2
    auto popVal = buff.popFront(); // 0
    ASSERT_TRUE(popVal.has_value());
    EXPECT_EQ(popVal.value(), expectedVal++);
    popVal = buff.popFront(); // 1
    ASSERT_TRUE(popVal.has_value());
    EXPECT_EQ(popVal.value(), expectedVal++);
    while (!buff.full()) {
        buff.pushBack(pushVal++); // should get up to 11, i.e., size - 1 + 2
    }
    while (!buff.empty()) {
        popVal = buff.popFront();
        ASSERT_TRUE(popVal.has_value());
        EXPECT_EQ(popVal.value(), expectedVal++);
    }
}

TEST(RingBufferTest, PushOverflowPop)
{
    constexpr size_t size = 10;
    RingBuffer<int, size> buff;
    int pushVal = 0;
    int expectedVal = size;
    while (size_t(pushVal) < size * 2) {
        buff.pushBack(pushVal++);
    }
    while (!buff.empty()) {
        auto popVal = buff.popFront();
        ASSERT_TRUE(popVal.has_value());
        EXPECT_EQ(popVal.value(), expectedVal++);
    }
    EXPECT_EQ(expectedVal, size * 2);
}

TEST(RingBufferTest, Size)
{
    constexpr size_t size = 10;
    RingBuffer<int, size> buff;
    int count = 0;
    // Make sure it works for normal operation (push)
    while (!buff.full()) {
        buff.pushBack(0);
        count++;
        EXPECT_EQ(buff.size(), count);
    }
    EXPECT_EQ(buff.size(), size);

    // Make sure it works for overflow
    buff.pushBack(0);
    EXPECT_EQ(buff.size(), size);

    // And back to normal operation (pop)
    while (!buff.empty()) {
        (void)buff.popFront();
        count--;
        EXPECT_EQ(buff.size(), count);
    }
    EXPECT_EQ(buff.size(), 0);

    // Make sure it works for underflow
    (void)buff.popFront();
    EXPECT_EQ(buff.size(), 0);
}