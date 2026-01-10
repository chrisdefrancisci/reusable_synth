#include <gmock/gmock.h>

#include <reusable_synth/hardware/led.hpp>

class MockLed : public ledBase {
public:
  MOCK_METHOD(void, on, (), (override));
  MOCK_METHOD(void, off, (), (override));
  MOCK_METHOD(void, setIntensity, (int value), (override));
  MOCK_METHOD(void, setIntensity, (float value), (override));
  MOCK_METHOD((std::pair<int, int>), getRange, (), (const, override));
};
