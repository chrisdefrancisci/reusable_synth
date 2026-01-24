#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include <reusable_synth/hardware/pin_change.hpp>

using PinChange = PinChangeBase<3>;

template<int multiplier>
class DerivedPinChange : public PinChange
{
public:
    DerivedPinChange(uint16_t pin)
      : PinChangeBase(pin)
      , data(pin)
    {
    }

private:
    void callback() override { data *= multiplier; }
    int data;
};

using ::testing::AtLeast;
using ::testing::AtMost;
using ::testing::Return;

TEST(PinChangeTest, registerPinOnConstruction)
{
    EXPECT_EQ(PinChange::getUsedCount(), 0);
    DerivedPinChange<2> pin2(0);
    EXPECT_EQ(PinChange::getUsedCount(), 1);
    EXPECT_TRUE(pin2.isUsed());
}

TEST(PinChangeTest, registerUntilFull)
{
    EXPECT_TRUE(false);
}

TEST(PinChangeTest, deregisterPinOnDestruction)
{
    EXPECT_TRUE(false);
}

TEST(PinChangeTest, deregisterThenRegister)
{
    EXPECT_TRUE(false);
}

TEST(PinChangeTest, dispatchCallsCallback)
{
    EXPECT_TRUE(false);
}

TEST(PinChangeTest, dispatchCallsMultipleCallbacks)
{
    EXPECT_TRUE(false);
}
