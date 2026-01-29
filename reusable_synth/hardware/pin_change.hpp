/**
 * @file pin_change.hpp
 * @author Chris DeFrancisci (chrisdefrancisci@gmail.com)
 * @brief Interface for EXTernal Interrupt/event - pin changes.
 * @date 2026-01-21
 */

#pragma once

#include <algorithm>
#include <cstdint>
#include <optional>
#include <span>

#include <reusable_synth/hardware/interrupt_handler.hpp>
#include <reusable_synth/utils/noncopyable.hpp>

namespace PinChange {

struct RegisteredPin
{
    int pin = 0;
    InterruptHandler handler;
    inline bool registered() const { return handler.isConnected(); }
};

using RegisteredPinSpan = std::span<RegisteredPin>;

/**
 * @brief
 *
 * @param pin
 * @return true
 * @return false
 */
inline bool isRegistered(const RegisteredPinSpan& registeredPins, int pin)
{
    auto it = std::find_if(registeredPins.begin(),
                           registeredPins.end(),
                           [pin](RegisteredPin registeredPin) {
                               return registeredPin.pin == pin &&
                                      registeredPin.registered();
                           });

    return it != registeredPins.end();
}

/**
 * @brief Get the Next Available object
 *
 * @param registeredPins
 * @return std::optional<RegisteredPin*>
 */
inline std::optional<RegisteredPin*> getNextAvailable(
  RegisteredPinSpan& registeredPins)
{
    auto it = std::find_if(
      registeredPins.begin(),
      registeredPins.end(),
      [](RegisteredPin registeredPin) { return !registeredPin.registered(); });
    if (it == registeredPins.end()) {
        return std::nullopt;
    }
    return std::make_optional(&(*it));
}

/**
 * @brief Registers each instance of a class derived from PinChangeBase.
 *
 * @param pin The GPIO pin associated with the interrupt.
 */
template<auto Func, typename Class>
bool registerPin(RegisteredPinSpan& registeredPins, int pin, Class* obj)
{
    if (isRegistered(registeredPins, pin)) {
        return false;
    }
    if (!getNextAvailable(registeredPins).has_value()) {
        return false;
    }

    auto registeredPin = getNextAvailable(registeredPins).value();
    registeredPin->pin = pin;
    registeredPin->handler.connect<Func>(obj);

    return true;
}

template<void (*Func)()>
bool registerPin(RegisteredPinSpan& registeredPins, int pin)
{
    if (isRegistered(registeredPins, pin)) {
        return false;
    }
    if (!getNextAvailable(registeredPins).has_value()) {
        return false;
    }

    auto registeredPin = getNextAvailable(registeredPins).value();
    registeredPin->pin = pin;
    // See https://en.cppreference.com/w/cpp/language/dependent_name.html
    // section, "the template disambiguator for dependent names"
    registeredPin->handler.template connect<Func>();

    return true;
}

/**
 * @brief Removes instance of class from registered list.
 *
 * @param pin Pin to deregister.
 */
inline bool deregisterPin(RegisteredPinSpan& registeredPins, int pin)
{
    auto it = std::find_if(
      registeredPins.begin(),
      registeredPins.end(),
      [pin](RegisteredPin registeredPin) { return registeredPin.pin == pin; });
    if (it != registeredPins.end()) {
        it->handler.disconnect();
        return true;
    }
    return false;
}

/**
 * @brief Calls the callback function associated with the pin.
 *
 * Use this function in interrupts.
 *
 * @param pin The pin associated with the pin change event.
 */
inline void irqDispatch(const RegisteredPinSpan& registeredPins, uint16_t pin)
{
    auto it = std::find_if(
      registeredPins.begin(),
      registeredPins.end(),
      [pin](RegisteredPin registeredPin) { return registeredPin.pin == pin; });

    if (it == registeredPins.end()) {
        return;
    }

    it->handler();
}

/**
 * @brief Get the number of used instances of the class.
 *
 * @return int
 */
inline int getUsedCount(const RegisteredPinSpan& registeredPins)
{
    int count = 0;
    for (const auto registeredPin : registeredPins) {
        if (registeredPin.registered()) {
            count++;
        }
    }
    return count;
}
}
