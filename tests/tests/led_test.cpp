#include "gmock/gmock.h"
#include <gtest/gtest.h>
#include <gmock/gmock.h>

#include <tests/mocks/MockLed.hpp>

using ::testing::AtLeast;
using ::testing::AtMost;
using ::testing::Return;

TEST(LedTest, test1)
{
    ASSERT_EQ(1, 1);
}

TEST(LedTest, test2)
{
    MockLed led;

    EXPECT_CALL(led, on()).Times(AtLeast(1));
    EXPECT_CALL(led, off()).Times(AtMost(0));
    EXPECT_CALL(led, getRange())
        .Times(AtLeast(1))
        .WillRepeatedly(Return(std::pair<int, int>(0, 1)));
    
    led.on();

    auto range = led.getRange();
    EXPECT_EQ(range.first, 0);
    EXPECT_EQ(range.second, 1);
}