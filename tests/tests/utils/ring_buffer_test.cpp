#include <gtest/gtest.h>

#include <reusable_synth/utils/ring_buffer.hpp>

TEST(RingBufferTest, EmptyBeforePush) {
  RingBuffer<int, 10> buff;
  auto val = buff.pop_front();
  EXPECT_FALSE(val.has_value());
}

TEST(RingBufferTest, SinglePushPop) {
  RingBuffer<int, 10> buff;
  int testVal = 1;
  buff.push_back(testVal);
  EXPECT_FALSE(buff.empty());
  auto val = buff.pop_front();
  ASSERT_TRUE(val.has_value());
  EXPECT_EQ(val.value(), testVal);
}

TEST(RingBufferTest, PushFillPopAll) {
  constexpr size_t size = 10;
  RingBuffer<int, size> buff;
  int i = 0;
  while (!buff.full()) {
    ASSERT_LT(i, size);
    buff.push_back(i);
    i++;
  }
  EXPECT_EQ(i, size);

  i = 0;
  while (!buff.empty()) {
    auto val = buff.pop_front();
    ASSERT_TRUE(val.has_value());
    EXPECT_EQ(val.value(), i);
    ASSERT_LT(i, size);
    i++;
  }
  EXPECT_FALSE(buff.pop_front().has_value());
}

TEST(RingBufferTest, UnevenPushPop) {
  constexpr size_t size = 10;
  RingBuffer<int, size> buff;
  int pushVal = 0;
  int expectedVal = 0;
  buff.push_back(pushVal++);      // 0
  buff.push_back(pushVal++);      // 1
  buff.push_back(pushVal++);      // 2
  auto popVal = buff.pop_front(); // 0
  ASSERT_TRUE(popVal.has_value());
  EXPECT_EQ(popVal.value(), expectedVal++);
  popVal = buff.pop_front(); // 1
  ASSERT_TRUE(popVal.has_value());
  EXPECT_EQ(popVal.value(), expectedVal++);
  while (!buff.full()) {
    buff.push_back(pushVal++); // should get up to 11, i.e., size - 1 + 2
  }
  while (!buff.empty()) {
    popVal = buff.pop_front();
    ASSERT_TRUE(popVal.has_value());
    EXPECT_EQ(popVal.value(), expectedVal++);
  }
}

TEST(RingBufferTest, PushOverflowPop) {
  constexpr size_t size = 10;
  RingBuffer<int, size> buff;
  int pushVal = 0;
  int expectedVal = size;
  while (pushVal < size * 2) {
    buff.push_back(pushVal++);
  }
  while (!buff.empty()) {
    auto popVal = buff.pop_front();
    ASSERT_TRUE(popVal.has_value());
    EXPECT_EQ(popVal.value(), expectedVal++);
  }
  EXPECT_EQ(expectedVal, size * 2);
}

TEST(RingBufferTest, Size) {
  constexpr size_t size = 10;
  RingBuffer<int, size> buff;
  int count = 0;
  // Make sure it works for normal operation (push)
  while (!buff.full()) {
    buff.push_back(0);
    count++;
    EXPECT_EQ(buff.size(), count);
  }
  EXPECT_EQ(buff.size(), size);

  // Make sure it works for overflow
  buff.push_back(0);
  EXPECT_EQ(buff.size(), size);

  // And back to normal operation (pop)
  while (!buff.empty()) {
    (void)buff.pop_front();
    count--;
    EXPECT_EQ(buff.size(), count);
  }
  EXPECT_EQ(buff.size(), 0);

  // Make sure it works for underflow
  (void)buff.pop_front();
  EXPECT_EQ(buff.size(), 0);
}