/**
 * @file pin_change.hpp
 * @author Chris DeFrancisci (chrisdefrancisci@gmail.com)
 * @brief Interface for EXTernal Interrupt/event - pin changes.
 *
 * @todo usage example
 * @date 2026-01-21
 */

#pragma once

#include <algorithm>
#include <cstdint>
#include <optional>
#include <span>

#include <reusable_synth/hardware/interrupt_handler.hpp>

namespace PinChange {

/**
 * @brief Helper class to hold a (pin, interrupt callback) pair.
 *
 */
struct RegisteredPin
{
    int pin = 0;
    InterruptHandler handler;
    [[nodiscard]] auto isRegistered() const -> bool
    {
        return handler.isConnected();
    }
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
inline auto is_registered(const RegisteredPinSpan& registeredPins, int pin)
  -> bool
{
    auto iter = std::ranges::find_if(
      registeredPins, [pin](RegisteredPin registeredPin) -> bool {
          return registeredPin.pin == pin && registeredPin.isRegistered();
      });

    return iter != registeredPins.end();
}

/**
 * @brief Get the next address to place a pin, if one is available.
 *
 * @param registeredPins The storage location of registered pins.
 * @return std::optional<RegisteredPin*> An address in registeredPins or
 * std::nullopt.
 */
inline auto get_next_available(RegisteredPinSpan& registeredPins)
  -> std::optional<RegisteredPin*>
{
    auto iter = std::ranges::find_if(registeredPins,
                                     [](RegisteredPin registeredPin) -> bool {
                                         return !registeredPin.isRegistered();
                                     });
    if (iter == registeredPins.end()) {
        return std::nullopt;
    }
    return std::make_optional(&(*iter));
}

/**
 * @brief Registers a pin interrupt to call a class method.
 *
 * @tparam Func The class method to call.
 * @tparam Class The type of object, can be implicitly determined.
 * @param registeredPins The storage location of registered pins.
 * @param pin The pin to register.
 * @param obj The object of type Class to call Func.
 * @return true If registration is successful.
 * @return false If registration is unsuccessful.
 */
template<auto Func, typename Class>
auto register_pin(RegisteredPinSpan& registeredPins, int pin, Class* obj)
  -> bool
{
    if (is_registered(registeredPins, pin)) {
        return false;
    }
    if (!get_next_available(registeredPins).has_value()) {
        return false;
    }

    auto* registeredPin = get_next_available(registeredPins).value();
    registeredPin->pin = pin;
    registeredPin->handler.connect<Func>(obj);

    return true;
}

/**
 * @brief Registers a pin interrupt to call a free function.
 *
 * @tparam (*Func)() The function to call.
 * @param registeredPins The storage location of registered pins.
 * @param pin The pin to register.
 * @return true If registration is successful.
 * @return false If registration is unsuccessful.
 */
template<void (*Func)()>
auto register_pin(RegisteredPinSpan& registeredPins, int pin) -> bool
{
    if (is_registered(registeredPins, pin)) {
        return false;
    }
    if (!get_next_available(registeredPins).has_value()) {
        return false;
    }

    auto* registeredPin = get_next_available(registeredPins).value();
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
inline auto deregister_pin(RegisteredPinSpan& registeredPins, int pin) -> bool
{
    auto iter = std::ranges::find_if(
      registeredPins, [pin](RegisteredPin registeredPin) -> bool {
          return registeredPin.pin == pin;
      });
    if (iter != registeredPins.end()) {
        iter->handler.disconnect();
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
    auto iter = std::ranges::find_if(
      registeredPins, [pin](RegisteredPin registeredPin) -> bool {
          return registeredPin.pin == pin;
      });

    if (iter == registeredPins.end()) {
        return;
    }

    iter->handler();
}

/**
 * @brief Get the number of used instances of the class.
 *
 * @return int
 */
inline auto get_used_count(const RegisteredPinSpan& registeredPins) -> int
{
    int count = 0;
    for (const auto registeredPin : registeredPins) {
        if (registeredPin.isRegistered()) {
            count++;
        }
    }
    return count;
}

/**
 * @brief Defines interface, particularly constructors, for pin change-based
 * interrupts.
 * @remark See <a
 * href="https://en.wikipedia.org/wiki/Curiously_recurring_template_pattern">CRTP
 * (Curiously Recurring Template Pattern)</a> for details on static
 * polymorphism.
 *
 * @tparam Derived The derived class, which implements the callback function.
 */
template<typename Derived>
class PinChangeInterface
{
public:
    PinChangeInterface(RegisteredPinSpan registeredPins, uint16_t pin)
      : registeredPins(registeredPins)
      , pin(pin)
      , registered(PinChange::register_pin<&Derived::callback, Derived>(
          registeredPins,
          pin,
          static_cast<Derived*>(this)))
    {
    }

    virtual ~PinChangeInterface()
    {
        if (registered) {
            PinChange::deregister_pin(registeredPins, pin);
        }
    }

    PinChangeInterface(const PinChangeInterface&) = delete; // Copy ctor
    auto operator=(const PinChangeInterface&)
      -> PinChangeInterface& = delete; // Copy assignment

    // "Move" registers one object and deregisters the other
    PinChangeInterface(PinChangeInterface&& other) noexcept
      : registeredPins(other.registeredPins)
      , pin(other.pin) // Move ctor
    {
        if (PinChange::deregister_pin(other.registeredPins, other.pin)) {
            // registeredPins = other.registeredPins;
            other.registered = false;
            // pin = other.pin;
            registered = PinChange::register_pin<&Derived::callback, Derived>(
              registeredPins, pin, static_cast<Derived*>(this));
        }
    }
    auto operator=(PinChangeInterface&& other) noexcept
      -> PinChangeInterface& // Move assignment
    {
        if (this != &other) {
            if (PinChange::deregister_pin(other.registeredPins, other.pin)) {
                registeredPins = other.registeredPins;
                other.registered = false;
                pin = other.pin;
                registered =
                  PinChange::register_pin<&Derived::callback, Derived>(
                    registeredPins, pin, static_cast<Derived*>(this));
            }
        }
        return *this;
    }
    [[nodiscard]] auto getPin() const -> int { return pin; }
    [[nodiscard]] auto isRegistered() const -> bool { return registered; }
    virtual void callback() = 0;

private:
    RegisteredPinSpan registeredPins;
    int pin;
    bool registered;
};

}
