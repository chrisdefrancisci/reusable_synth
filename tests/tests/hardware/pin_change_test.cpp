#include <gtest/gtest.h>

#include <reusable_synth/hardware/pin_change.hpp>
#include <reusable_synth/utils/noncopyable.hpp>
#include <vector>

static constexpr int pinCount = 3;

class TestPinChange : Noncopyable
{
public:
    TestPinChange(uint16_t pin,
                  std::span<PinChange::RegisteredPin> registeredPins)
      : registeredPins(registeredPins)
      , pin(pin)
      , count(0)
      , registered(
          PinChange::register_pin<&TestPinChange::callback>(registeredPins,
                                                            pin,
                                                            this))
    {
    }

    // TODO create a "peripheral base class" or something
    // Following "rule of 5", if dtor is defined, the rest of these should be
    // defined too
    virtual ~TestPinChange()
    {
        if (registered) {
            PinChange::deregister_pin(registeredPins, pin);
        }
    }
    TestPinChange(const TestPinChange&) = delete;            // Copy ctor
    TestPinChange& operator=(const TestPinChange&) = delete; // Copy assignment

    // "Move" registers one object and deregisters the other
    TestPinChange(TestPinChange&& other) noexcept // Move ctor
    {
        PinChange::deregister_pin(other.registeredPins, other.pin);
        registeredPins = other.registeredPins;
        other.registered = false;
        pin = other.pin;
        count = 0;
        registered = PinChange::register_pin<&TestPinChange::callback>(
          registeredPins, pin, this);
    }
    TestPinChange& operator=(TestPinChange&& other) noexcept // Move assignment
    {
        if (this != &other) {
            PinChange::deregister_pin(other.registeredPins, other.pin);
            registeredPins = other.registeredPins;
            other.registered = false;
            pin = other.pin;
            count = 0;
            registered = PinChange::register_pin<&TestPinChange::callback>(
              registeredPins, pin, this);
        }
        return *this;
    }
    int getCount() const { return count; }
    int getPin() const { return pin; }
    bool isRegistered() const { return registered; }

private:
    void callback() { count++; }
    std::span<PinChange::RegisteredPin> registeredPins;
    int pin;
    int count;
    bool registered;
};

TEST(PinChangeTest, registerPinOnConstruction)
{
    std::array<PinChange::RegisteredPin, pinCount> registeredPins;
    int pinNum = 0;
    EXPECT_EQ(PinChange::get_used_count(registeredPins), 0);
    TestPinChange pin(pinNum, registeredPins);
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
        pins.push_back(TestPinChange(i, registeredPins));
        // pins.emplace_back(i, registeredPins);
        EXPECT_TRUE(pins.back().isRegistered());
        EXPECT_TRUE(PinChange::is_registered(registeredPins, i));
    }
    EXPECT_EQ(PinChange::get_used_count(registeredPins), pinCount);
    // No more objects can be registered
    pins.push_back(TestPinChange(pinCount, registeredPins));
    EXPECT_EQ(PinChange::get_used_count(registeredPins), pinCount);
    EXPECT_FALSE(
      PinChange::is_registered(registeredPins, pins.back().getPin()));
}

TEST(PinChangeTest, dontRegisterDuplicatePins)
{
    std::array<PinChange::RegisteredPin, pinCount> registeredPins;
    int pinNum = 0;
    EXPECT_EQ(PinChange::get_used_count(registeredPins), 0);
    TestPinChange pinOriginal(pinNum, registeredPins);
    EXPECT_EQ(PinChange::get_used_count(registeredPins), 1);
    EXPECT_TRUE(PinChange::is_registered(registeredPins, pinNum));
    TestPinChange pinDuplicate(pinNum, registeredPins);
    EXPECT_EQ(PinChange::get_used_count(registeredPins), 1);
    EXPECT_FALSE(pinDuplicate.isRegistered());
}
TEST(PinChangeTest, deregisterPinOnDestruction)
{
    std::array<PinChange::RegisteredPin, pinCount> registeredPins;
    int pinNum = 0;
    EXPECT_EQ(PinChange::get_used_count(registeredPins), 0);
    auto pinToBeDeleted = new TestPinChange(pinNum, registeredPins);
    EXPECT_EQ(PinChange::get_used_count(registeredPins), 1);
    delete pinToBeDeleted;
    EXPECT_EQ(PinChange::get_used_count(registeredPins), 0);
    // Ensure deregistration is completed by registering that pin again
    TestPinChange pinReregistered(pinNum, registeredPins);
    EXPECT_EQ(PinChange::get_used_count(registeredPins), 1);
    EXPECT_TRUE(pinReregistered.isRegistered());
}

TEST(PinChangeTest, dispatchCallsCallback)
{
    std::array<PinChange::RegisteredPin, pinCount> registeredPins;
    int pinNum = 0;
    TestPinChange pin(pinNum, registeredPins);
    EXPECT_EQ(pin.getCount(), 0);
    PinChange::irq_dispatch(registeredPins, pinNum);
    EXPECT_EQ(pin.getCount(), 1);
}

TEST(PinChangeTest, dispatchCallsMultipleCallbacks)
{

    std::array<PinChange::RegisteredPin, pinCount> registeredPins;
    std::vector<TestPinChange> pins;
    for (int i = 0; i < pinCount; i++) {
        pins.push_back(TestPinChange(i, registeredPins));
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
