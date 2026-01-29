/**
 * @file debounced_button.hpp
 * @author Chris DeFrancisci (chrisdefrancisci@gmail.com)
 * @brief
 * @date 2026-01-25
 */

#pragma once

#include <chrono>
#include <functional>
#include <memory>

#include <ratio>
#include <reusable_synth/hardware/pin_change.hpp>

template<int N>
class DebouncedButtonEdge
{
public:
    DebouncedButtonEdge(int pin, PinChange::RegisteredPinSpan registeredPins)
      : pin(pin)
    {
    }
    virtual ~DebouncedButtonEdge() {}
    void registerRisingEdgeCallback(std::function<void()> cb) {}
    void registerFallingEdgeCallback(std::function<void()> cb) {}
    std::chrono::milliseconds getTimeSinceEdge() const { return {}; }

private:
    int pin;
    bool registered;
};

class DebouncedButtonBidirectional
{
public:
    void registerRisingEdgeCallback(std::function<void()> cb) {}
    void registerFallingEdgeCallback(std::function<void()> cb) {}
    std::chrono::milliseconds getTimeSinceEdge() const { return {}; }

private:
};
