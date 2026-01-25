#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include <memory>
#include <reusable_synth/hardware/pin_change.hpp>
#include <vector>

static constexpr int pinCount = 3;
using PinChange = PinChangeBase<pinCount>;

class DerivedPinChange : public PinChange
{
public:
    DerivedPinChange(uint16_t pin)
      : PinChangeBase(pin)
      , count(0)
    {
    }
    int getCount() const { return count; }

private:
    void callback() override { count++; }
    int count;
};

TEST(PinChangeTest, registerPinOnConstruction)
{
    EXPECT_EQ(PinChange::getUsedCount(), 0);
    DerivedPinChange pin(0);
    EXPECT_EQ(PinChange::getUsedCount(), 1);
    EXPECT_TRUE(pin.isRegistered());
}

TEST(PinChangeTest, registerUntilFull)
{
    std::vector<std::unique_ptr<PinChange>> pins;
    for (int i = 0; i < pinCount; i++) {
        EXPECT_EQ(PinChange::getUsedCount(), i);
        pins.push_back(std::make_unique<DerivedPinChange>(i));
        EXPECT_TRUE(pins.back()->isRegistered());
    }
    EXPECT_EQ(PinChange::getUsedCount(), pinCount);
    // No more objects can be registered
    pins.push_back(std::make_unique<DerivedPinChange>(pinCount));
    EXPECT_EQ(PinChange::getUsedCount(), pinCount);
    EXPECT_FALSE(pins.back()->isRegistered());
}

TEST(PinChangeTest, dontRegisterDuplicatePins)
{
    EXPECT_EQ(PinChange::getUsedCount(), 0);
    DerivedPinChange pinOriginal(0);
    EXPECT_EQ(PinChange::getUsedCount(), 1);
    EXPECT_TRUE(pinOriginal.isRegistered());
    DerivedPinChange pinDuplicate(0);
    EXPECT_EQ(PinChange::getUsedCount(), 1);
    EXPECT_FALSE(pinDuplicate.isRegistered());
}
TEST(PinChangeTest, deregisterPinOnDestruction)
{
    int pinNum = 0;
    EXPECT_EQ(PinChange::getUsedCount(), 0);
    auto pinToBeDeleted = new DerivedPinChange(pinNum);
    EXPECT_EQ(PinChange::getUsedCount(), 1);
    delete pinToBeDeleted;
    EXPECT_EQ(PinChange::getUsedCount(), 0);
    // Ensure deregistration is completed by registering that pin again
    DerivedPinChange pinReregistered(pinNum);
    EXPECT_EQ(PinChange::getUsedCount(), 1);
    EXPECT_TRUE(pinReregistered.isRegistered());
}

TEST(PinChangeTest, dispatchCallsCallback)
{
    int pinNum = 0;
    DerivedPinChange pin(pinNum);
    EXPECT_EQ(pin.getCount(), 0);
    PinChange::irqDispatch(pinNum);
    EXPECT_EQ(pin.getCount(), 1);
}

TEST(PinChangeTest, dispatchCallsMultipleCallbacks)
{
    std::vector<std::unique_ptr<PinChange>> pins;
    for (int i = 0; i < pinCount; i++) {
        pins.push_back(std::make_unique<DerivedPinChange>(i));
    }

    // Call each callback some different number of times (pin # + 1)
    for (int pin = 0; pin < pinCount; pin++) {
        for (int i = 0; i <= pin; i++) {
            PinChange::irqDispatch(pin);
        }
    }

    int i = 0;
    for (const auto& pin : pins) {
        i++;
        EXPECT_EQ(static_cast<const DerivedPinChange*>(pin.get())->getCount(),
                  i);
    }
}
