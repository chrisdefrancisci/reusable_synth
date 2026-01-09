#include <gtest/gtest.h>
#include <reusable_synth/Utils/ring_buffer.hpp>
#include <string>

TEST(RingBufferTest, BasicWriteAndRead)
{
    RingBuffer<int, 10> rb;

    EXPECT_TRUE(rb.isEmpty());

    rb.write(10);
    rb.write(20);
    rb.write(30);

    EXPECT_FALSE(rb.isEmpty());
    EXPECT_EQ(rb.read(), 10);
    EXPECT_EQ(rb.read(), 20);
    EXPECT_EQ(rb.read(), 30);
    EXPECT_TRUE(rb.isEmpty());

    EXPECT_THROW(rb.read(), std::runtime_error);
}

TEST(RingBufferTest, OverwriteWhenFull)
{
    RingBuffer<int, 5> rb;

    // Fill buffer
    for (int i = 0; i < 4; i++) {
        rb.write(i);
    }

    EXPECT_TRUE(rb.isFull());

    // Write more - should overwrite oldest
    rb.write(100);
    rb.write(101);

    // Should have: 2, 3, 100, 101 (0 and 1 were overwritten)
    EXPECT_EQ(rb.read(), 2);
    EXPECT_EQ(rb.read(), 3);
    EXPECT_EQ(rb.read(), 100);
    EXPECT_EQ(rb.read(), 101);
}

TEST(RingBufferTest, StringLogs)
{
    RingBuffer<std::string, 5> logs;

    // Write more than capacity
    for (int i = 0; i < 10; i++) {
        logs.write("Log " + std::to_string(i));
    }

    // Should keep only last 4 entries
    EXPECT_EQ(logs.read(), "Log 6");
    EXPECT_EQ(logs.read(), "Log 7");
    EXPECT_EQ(logs.read(), "Log 8");
    EXPECT_EQ(logs.read(), "Log 9");
    EXPECT_TRUE(logs.isEmpty());
}
