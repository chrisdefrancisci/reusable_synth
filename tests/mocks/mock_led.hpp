#include <gmock/gmock.h>

#include <reusable_synth/hardware/led_interface.hpp>

class MockLed : public LedInterface
{
public:
    MOCK_METHOD(void, on, (), (override));
    MOCK_METHOD(void, off, (), (override));
    MOCK_METHOD(void, setIntensity, (int), (override));
    MOCK_METHOD(void, setIntensity, (float), (override));
    MOCK_METHOD((std::pair<int, int>), getRange, (), (const, override));
};
