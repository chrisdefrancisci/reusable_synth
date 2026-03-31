#include <chrono>
#include <vector>

#include <gtest/gtest.h>
#include <reusable_synth/hardware/pin_change.hpp>
#include <reusable_synth/middleware/debounced_button.hpp>
#include <tests/stubs/stub_getter.hpp>

using Millis = std::chrono::duration<uint32_t, std::milli>;
static Millis (*getTime)() = get_free_function_value<Millis>;
static Millis (*setTime)(std::optional<Millis>) =
  set_free_function_value<Millis>;

static constexpr int pinCount = 4;

static int mockFreeCallbackCounter = 0;
static void mock_free_callback()
{
    mockFreeCallbackCounter++;
}

class MockCount
{
public:
    void callback() { count++; }
    [[nodiscard]] auto getCount() const -> int { return count; }

private:
    int count = 0;
};

TEST(DebouncedButtonTest, edgeCallbackCalled)
{
    ASSERT_EQ(setTime(Millis(0)), Millis(0));
    std::array<PinChange::RegisteredPin, pinCount> registeredPins;
    constexpr Millis debounceTime{ 200 };
    ASSERT_EQ(setTime(debounceTime), debounceTime);

    std::vector<DebouncedButtonEdge<Millis>> buttons;
    for (int pinNum = 0; pinNum < pinCount; pinNum++) {
        buttons.emplace_back(registeredPins, pinNum, getTime, debounceTime);
    }

    buttons[0].registerEdgeCallback(mock_free_callback);

    MockCount countInstance;
    buttons[1].registerEdgeCallback(
      [&countInstance]() -> void { countInstance.callback(); });

    int localCount = 0;
    buttons[2].registerEdgeCallback([&localCount]() -> void { localCount++; });

    mockFreeCallbackCounter = 0;
    ASSERT_EQ(countInstance.getCount(), 0);
    EXPECT_EQ(PinChange::get_used_count(registeredPins), pinCount);

    // call each callback a different number of times
    // Time elapsed IS enough to call the same callback
    for (const auto& button : buttons) {
        for (int i = 0; i <= button.getPin(); i++) {
            setTime(getTime() + debounceTime);
            PinChange::irq_dispatch(registeredPins, button.getPin());
        }
    }

    EXPECT_EQ(mockFreeCallbackCounter, 1);
    EXPECT_EQ(countInstance.getCount(), 2);
    EXPECT_EQ(localCount, 3);
}

TEST(DebouncedButtonTest, edgeCallbackDebounced)
{
    ASSERT_EQ(setTime(Millis(0)), Millis(0));
    std::array<PinChange::RegisteredPin, pinCount> registeredPins;
    constexpr Millis debounceTime{ 200 };
    ASSERT_EQ(setTime(debounceTime), debounceTime);

    std::vector<DebouncedButtonEdge<Millis>> buttons;
    for (int pinNum = 0; pinNum < pinCount; pinNum++) {
        buttons.emplace_back(registeredPins, pinNum, getTime, debounceTime);
    }

    buttons[0].registerEdgeCallback(mock_free_callback);

    MockCount countInstance;
    buttons[1].registerEdgeCallback(
      [&countInstance]() -> void { countInstance.callback(); });

    int localCount = 0;
    buttons[2].registerEdgeCallback([&localCount]() -> void { localCount++; });

    mockFreeCallbackCounter = 0;
    ASSERT_EQ(countInstance.getCount(), 0);
    EXPECT_EQ(PinChange::get_used_count(registeredPins), pinCount);

    // call each callback a different number of times
    // Time elapsed is NOT enough to call the same callback
    for (const auto& button : buttons) {
        for (int i = 0; i <= button.getPin(); i++) {
            setTime(getTime() + Millis(1));
            PinChange::irq_dispatch(registeredPins, button.getPin());
        }
    }

    EXPECT_EQ(mockFreeCallbackCounter, 1);
    EXPECT_EQ(countInstance.getCount(), 1);
    EXPECT_EQ(localCount, 1);
}