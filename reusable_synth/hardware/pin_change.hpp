/**
 * @file pin_change.hpp
 * @author Chris DeFrancisci (chrisdefrancisci@gmail.com)
 * @brief Interface for EXTernal Interrupt/event - pin changes.
 * @todo documentation!
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
    inline bool isRegistered() const { return handler.isConnected(); }
};

using RegisteredPinSpan = std::span<RegisteredPin>;

/**
 * @brief Checks registration for a pin.
 *
 * @param registeredPins The storage location of registered pins.
 * @param pin The pin to check.
 * @return true if pin is in the set and has an active interrupt handler.
 * @return false if pin is not in the set or doesn't have an active interrupt
 * handler.
 */
inline bool is_registered(const RegisteredPinSpan& registeredPins, int pin)
{
    auto it = std::find_if(registeredPins.begin(),
                           registeredPins.end(),
                           [pin](RegisteredPin registeredPin) {
                               return registeredPin.pin == pin &&
                                      registeredPin.isRegistered();
                           });

    return it != registeredPins.end();
}

/**
 * @brief Get the next address to place a pin, if one is available.
 *
 * @param registeredPins The storage location of registered pins.
 * @return std::optional<RegisteredPin*> An address in registeredPins or
 * std::nullopt.
 */
inline std::optional<RegisteredPin*> get_next_available(
  RegisteredPinSpan& registeredPins)
{
    auto it = std::find_if(registeredPins.begin(),
                           registeredPins.end(),
                           [](RegisteredPin registeredPin) {
                               return !registeredPin.isRegistered();
                           });
    if (it == registeredPins.end()) {
        return std::nullopt;
    }
    return std::make_optional(&(*it));
}

/**
 * @brief
 *
 * @tparam Func
 * @tparam Class
 * @param registeredPins
 * @param pin
 * @param obj
 * @return true
 * @return false
 */
template<auto Func, typename Class>
bool register_pin(RegisteredPinSpan& registeredPins, int pin, Class* obj)
{
    if (is_registered(registeredPins, pin)) {
        return false;
    }
    if (!get_next_available(registeredPins).has_value()) {
        return false;
    }

    auto registeredPin = get_next_available(registeredPins).value();
    registeredPin->pin = pin;
    registeredPin->handler.connect<Func>(obj);

    return true;
}

/**
 * @brief
 *
 * @tparam (*Func)()
 * @param registeredPins
 * @param pin
 * @return true
 * @return false
 */
template<void (*Func)()>
bool register_pin(RegisteredPinSpan& registeredPins, int pin)
{
    if (is_registered(registeredPins, pin)) {
        return false;
    }
    if (!get_next_available(registeredPins).has_value()) {
        return false;
    }

    auto registeredPin = get_next_available(registeredPins).value();
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
inline bool deregister_pin(RegisteredPinSpan& registeredPins, int pin)
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
inline void irq_dispatch(const RegisteredPinSpan& registeredPins, uint16_t pin)
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
inline int get_used_count(const RegisteredPinSpan& registeredPins)
{
    int count = 0;
    for (const auto registeredPin : registeredPins) {
        if (registeredPin.isRegistered()) {
            count++;
        }
    }
    return count;
}
}
