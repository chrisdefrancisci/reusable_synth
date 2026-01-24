#include <gmock/gmock.h>

#include <reusable_synth/hardware/pin_change.hpp>

template<int pinChangeCount>
class MockPinChange : public PinChangeBase<pinChangeCount>
{
public:
    MockPinChange()
      : PinChangeBase<pinChangeCount>()
    {
    }
    MOCK_METHOD(void, callback, (), (override));
};