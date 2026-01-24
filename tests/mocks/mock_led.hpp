#include <gmock/gmock.h>

#include <reusable_synth/hardware/led.hpp>

class MockLed : public LedBase {
public:
  MOCK_METHOD(void, on, (), (override));
  MOCK_METHOD(void, off, (), (override));
  MOCK_METHOD((std::pair<int, int>), getRange, (), (const, override));
};
