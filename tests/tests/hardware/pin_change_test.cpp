#include <gtest/gtest.h>

#include <reusable_synth/hardware/pin_change.hpp>
#include <vector>

static constexpr int pinCount = 3;

class TestPinChange
{
public:
    TestPinChange(uint16_t pin,
                  std::span<PinChange::RegisteredPin> registeredPins)
      : registeredPins(registeredPins)
      , pin(pin)
      , count(0)
      , registered(
          PinChange::registerPin<&TestPinChange::callback>(registeredPins,
                                                           pin,
                                                           this))
    {
    }
    virtual ~TestPinChange() { PinChange::deregisterPin(registeredPins, pin); }
    int getCount() const { return count; }
    int getPin() const { return pin; }
    bool isRegistered() const { return registered; }

private:
    void callback() { count++; }
    std::span<PinChange::RegisteredPin> registeredPins;
    int pin;
    int count;
    const bool registered;
};

TEST(PinChangeTest, registerPinOnConstruction)
{
    std::array<PinChange::RegisteredPin, pinCount> registeredPins;
    int pinNum = 0;
    EXPECT_EQ(PinChange::getUsedCount(registeredPins), 0);
    TestPinChange pin(pinNum, registeredPins);
    EXPECT_EQ(PinChange::getUsedCount(registeredPins), 1);
    EXPECT_TRUE(PinChange::isRegistered(registeredPins, pinNum));
}

TEST(PinChangeTest, registerUntilFull)
{
    std::array<PinChange::RegisteredPin, pinCount> registeredPins;
    // Note that in real usage, we shouldn't regularly need a container of
    // objects that point to different usages of pin changes. E.g., why combine
    // a UI button press and a peripheral data ready flag?
    std::vector<TestPinChange> pins;
    for (int i = 0; i < pinCount; i++) {
        EXPECT_EQ(PinChange::getUsedCount(registeredPins), i);
        pins.push_back(TestPinChange(i, registeredPins));
        EXPECT_TRUE(PinChange::isRegistered(registeredPins, i));
    }
    EXPECT_EQ(PinChange::getUsedCount(registeredPins), pinCount);
    // No more objects can be registered
    pins.push_back(TestPinChange(pinCount, registeredPins));
    EXPECT_EQ(PinChange::getUsedCount(registeredPins), pinCount);
    EXPECT_FALSE(PinChange::isRegistered(registeredPins, pins.back().getPin()));
}

TEST(PinChangeTest, dontRegisterDuplicatePins)
{
    std::array<PinChange::RegisteredPin, pinCount> registeredPins;
    int pinNum = 0;
    EXPECT_EQ(PinChange::getUsedCount(registeredPins), 0);
    TestPinChange pinOriginal(pinNum, registeredPins);
    EXPECT_EQ(PinChange::getUsedCount(registeredPins), 1);
    EXPECT_TRUE(PinChange::isRegistered(registeredPins, pinNum));
    TestPinChange pinDuplicate(pinNum, registeredPins);
    EXPECT_EQ(PinChange::getUsedCount(registeredPins), 1);
    EXPECT_FALSE(pinDuplicate.isRegistered());
}
TEST(PinChangeTest, deregisterPinOnDestruction)
{
    EXPECT_TRUE(false);
    // int pinNum = 0;
    // EXPECT_EQ(PinChangeManager::getUsedCount(), 0);
    // auto pinToBeDeleted = new DerivedPinChange(pinNum);
    // EXPECT_EQ(PinChangeManager::getUsedCount(), 1);
    // delete pinToBeDeleted;
    // EXPECT_EQ(PinChangeManager::getUsedCount(), 0);
    // // Ensure deregistration is completed by registering that pin again
    // DerivedPinChange pinReregistered(pinNum);
    // EXPECT_EQ(PinChangeManager::getUsedCount(), 1);
    // EXPECT_TRUE(pinReregistered.isRegistered());
}

TEST(PinChangeTest, dispatchCallsCallback)
{
    EXPECT_TRUE(false);
    // int pinNum = 0;
    // DerivedPinChange pin(pinNum);
    // EXPECT_EQ(pin.getCount(), 0);
    // PinChangeManager::irqDispatch(pinNum);
    // EXPECT_EQ(pin.getCount(), 1);
}

TEST(PinChangeTest, dispatchCallsMultipleCallbacks)
{
    EXPECT_TRUE(false);
    // std::vector<std::unique_ptr<PinChangeManager>> pins;
    // for (int i = 0; i < pinCount; i++) {
    //     pins.push_back(std::make_unique<DerivedPinChange>(i));
    // }

    // // Call each callback some different number of times (pin # + 1)
    // for (int pin = 0; pin < pinCount; pin++) {
    //     for (int i = 0; i <= pin; i++) {
    //         PinChangeManager::irqDispatch(pin);
    //     }
    // }

    // int i = 0;
    // for (const auto& pin : pins) {
    //     i++;
    //     EXPECT_EQ(static_cast<const
    //     DerivedPinChange*>(pin.get())->getCount(),
    //               i);
    // }
}
