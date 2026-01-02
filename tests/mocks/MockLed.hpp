#include <gmock/gmock.h>

#include <reusable_synth/Hardware/led.hpp>


class MockLed : public ledBase
{
    MOCK_METHOD(void, on, (), (override));
};
