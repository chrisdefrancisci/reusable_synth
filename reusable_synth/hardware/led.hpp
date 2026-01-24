/**
 * @file led.hpp
 * @author Chris DeFrancisci (chrisdefrancisci@gmail.com)
 * @brief Defines interfaces for controlling LEDs.
 * @todo Interface for "analog LED" that could be implemented with PWM or DAC.
 * @date 2025-12-27
 */

#pragma once

// Standard library includes
#include <utility>

/**
 * @brief This is the abstract interface class for writing to an LED.
 *
 * @todo This should be moved to reusable_synth/Hardware
 */
class LedBase
{
public:
    virtual ~LedBase() = default;
    /**
     * @brief Turns LED on.
     */
    virtual void on() = 0;

    /**
     * @brief Turns LED off.
     */
    virtual void off() = 0;

    /**
     * @brief Get the range of the LED intensity
     *
     * @return std::pair<int, int>
     */
    virtual std::pair<int, int> getRange() const = 0;

protected:
    LedBase() = default;
};

/**
 * @brief A template derived class of ledBase that must be specialized for the
 * hardware.
 *
 * @tparam T The type of the hardware handle.
 */
template<typename T>
class Led : public LedBase
{
private:
    Led() = default;
};
