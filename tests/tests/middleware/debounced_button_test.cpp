#include "pin_change.hpp"
#include <chrono>
#include <vector>

#include <gtest/gtest.h>
#include <reusable_synth/middleware/debounced_button.hpp>
#include <tests/stubs/stub_getter.hpp>

using millis = std::chrono::duration<uint32_t, std::milli>;
static millis (*getTime)() = getFreeFunctionValue<millis>;
static millis (*setTime)(std::optional<millis>) = setFreeFunctionValue<millis>;

static constexpr int pinCount = 4;

static int mock_free_callback_counter = 0;
static void mock_free_callback()
{
    mock_free_callback_counter++;
}

class MockCount
{
public:
    void callback() {}
    int getCount() { return count; }

private:
    int count = 0;
};

TEST(DebouncedButtonTest, edgeCallbackCalled)
{
    ASSERT_EQ(setTime(millis(0)), millis(0));
    std::array<PinChange::RegisteredPin, pinCount> registeredPins;
    millis debounceTime = millis(200);
    ASSERT_EQ(setTime(debounceTime), debounceTime);

    std::vector<DebouncedButtonEdge<millis>> buttons;
    for (int pinNum = 0; pinNum < pinCount; pinNum++) {
        // buttons.push_back(DebouncedButtonEdge<millis>(
        buttons.push_back(
          DebouncedButtonEdge(registeredPins, pinNum, getTime, debounceTime));
    }
    buttons[0].registerEdgeCallback(mock_free_callback);
    MockCount countInstance;
    buttons[1].registerEdgeCallback(
      [&countInstance]() { countInstance.callback(); });

    ASSERT_EQ(mock_free_callback_counter, 0);
    ASSERT_EQ(countInstance.getCount(), 0);
    EXPECT_EQ(PinChange::get_used_count(registeredPins), pinCount);

    // call each callback a different number of times
    for (const auto& button : buttons) {
        for (int i = 1; i <= button.getPin(); i++) {
            PinChange::irq_dispatch(registeredPins, button.getPin());
        }
    }

    EXPECT_EQ(mock_free_callback_counter, 1);
    EXPECT_EQ(countInstance.getCount(), 2);
}

TEST(DebouncedButtonTest, edgeCallbackDebounced)
{
    EXPECT_TRUE(false);
}