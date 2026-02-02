#include <array>

#include <gtest/gtest.h>
#include <reusable_synth/hardware/pin_change.hpp>
#include <reusable_synth/utils/noncopyable.hpp>

static constexpr int pinCount = 3;

class TestPinChange : public PinChange::PinChangeInterface<TestPinChange>
{
public:
    TestPinChange(PinChange::RegisteredPinSpan registeredPins, uint16_t pin)
      : PinChange::PinChangeInterface<TestPinChange>(registeredPins, pin)
      , count(0)
    {
    }

    int getCount() const { return count; }
    virtual void callback() override { count++; }

private:
    int count;
};

TEST(PinChangeTest, registerPinOnConstruction)
{
    std::array<PinChange::RegisteredPin, pinCount> registeredPins;
    int pinNum = 0;
    EXPECT_EQ(PinChange::get_used_count(registeredPins), 0);
    TestPinChange pin(registeredPins, pinNum);
    EXPECT_EQ(PinChange::get_used_count(registeredPins), 1);
    EXPECT_TRUE(PinChange::is_registered(registeredPins, pinNum));
}

TEST(PinChangeTest, registerUntilFull)
{
    std::array<PinChange::RegisteredPin, pinCount> registeredPins;
    // Note that in real usage, we shouldn't regularly need a container of
    // objects that point to different usages of pin changes. E.g., why combine
    // a UI button press and a peripheral data ready flag?
    std::vector<TestPinChange> pins;
    for (int i = 0; i < pinCount; i++) {
        EXPECT_EQ(PinChange::get_used_count(registeredPins), i);
        pins.push_back(TestPinChange(registeredPins, i));
        // pins.emplace_back(registeredPins, i);
        EXPECT_TRUE(pins.back().isRegistered());
        EXPECT_TRUE(PinChange::is_registered(registeredPins, i));
    }
    EXPECT_EQ(PinChange::get_used_count(registeredPins), pinCount);
    // No more objects can be registered
    pins.push_back(TestPinChange(registeredPins, pinCount));
    EXPECT_EQ(PinChange::get_used_count(registeredPins), pinCount);
    EXPECT_FALSE(
      PinChange::is_registered(registeredPins, pins.back().getPin()));
}

TEST(PinChangeTest, dontRegisterDuplicatePins)
{
    std::array<PinChange::RegisteredPin, pinCount> registeredPins;
    int pinNum = 0;
    EXPECT_EQ(PinChange::get_used_count(registeredPins), 0);
    TestPinChange pinOriginal(registeredPins, pinNum);
    EXPECT_EQ(PinChange::get_used_count(registeredPins), 1);
    EXPECT_TRUE(PinChange::is_registered(registeredPins, pinNum));
    TestPinChange pinDuplicate(registeredPins, pinNum);
    EXPECT_EQ(PinChange::get_used_count(registeredPins), 1);
    EXPECT_FALSE(pinDuplicate.isRegistered());
}
TEST(PinChangeTest, deregisterPinOnDestruction)
{
    std::array<PinChange::RegisteredPin, pinCount> registeredPins;
    int pinNum = 0;
    EXPECT_EQ(PinChange::get_used_count(registeredPins), 0);
    auto pinToBeDeleted = new TestPinChange(registeredPins, pinNum);
    EXPECT_EQ(PinChange::get_used_count(registeredPins), 1);
    delete pinToBeDeleted;
    EXPECT_EQ(PinChange::get_used_count(registeredPins), 0);
    // Ensure deregistration is completed by registering that pin again
    TestPinChange pinReregistered(registeredPins, pinNum);
    EXPECT_EQ(PinChange::get_used_count(registeredPins), 1);
    EXPECT_TRUE(pinReregistered.isRegistered());
}

TEST(PinChangeTest, dispatchCallsCallback)
{
    std::array<PinChange::RegisteredPin, pinCount> registeredPins;
    int pinNum = 0;
    TestPinChange pin(registeredPins, pinNum);
    EXPECT_EQ(pin.getCount(), 0);
    PinChange::irq_dispatch(registeredPins, pinNum);
    EXPECT_EQ(pin.getCount(), 1);
}

TEST(PinChangeTest, dispatchCallsMultipleCallbacks)
{

    std::array<PinChange::RegisteredPin, pinCount> registeredPins;
    std::vector<TestPinChange> pins;
    for (int i = 0; i < pinCount; i++) {
        pins.push_back(TestPinChange(registeredPins, i));
    }

    // Call each callback some different number of times (pin # + 1)
    for (int pin = 0; pin < pinCount; pin++) {
        for (int i = 0; i <= pin; i++) {
            PinChange::irq_dispatch(registeredPins, pin);
        }
    }

    int i = 0;
    for (const auto& pin : pins) {
        i++;
        EXPECT_EQ(pin.getCount(), i);
    }
}
