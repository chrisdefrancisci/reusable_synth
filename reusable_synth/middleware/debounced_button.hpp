/**
 * @file debounced_button.hpp
 * @author Chris DeFrancisci (chrisdefrancisci@gmail.com)
 * @brief
 * @date 2026-01-25
 */

#pragma once

#include <functional>

#include <reusable_synth/hardware/pin_change.hpp>
#include <reusable_synth/software/task.hpp>

/**
 * @brief
 *
 * Usage: call PinChange::irq_dispatch(pin) in the EXTI callback you wish to
 * debounce.
 *
 * @tparam TickType
 */
template<typename TickType>
class DebouncedButtonEdge
  : public PinChange::PinChangeInterface<DebouncedButtonEdge<TickType>>
{
public:
    DebouncedButtonEdge(PinChange::RegisteredPinSpan registeredPins,
                        uint16_t pin,
                        Timer<TickType>::TickFuncType getTick,
                        TickType debounceInterval)
      : PinChange::PinChangeInterface<DebouncedButtonEdge<TickType>>(
          registeredPins,
          pin)
      , debounceTimer(getTick)
      , debounceInterval(debounceInterval)
      , edgeCallback(nullptr)
    {
    }

    void registerEdgeCallback(std::function<void()> cb) { edgeCallback = cb; }
    virtual void callback() override
    {
        if (debounceTimer.timeout() && edgeCallback != nullptr) {
            debounceTimer.startInterval(debounceInterval);
            edgeCallback();
        }
    }

private:
    Timer<TickType> debounceTimer;
    TickType debounceInterval;
    std::function<void()> edgeCallback;
};
