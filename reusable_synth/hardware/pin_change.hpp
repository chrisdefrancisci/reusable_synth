/**
 * @file pin_change.hpp
 * @author Chris DeFrancisci (chrisdefrancisci@gmail.com)
 * @brief Interface for EXTernal Interrupt/event - pin changes.
 * @date 2026-01-21
 */

#pragma once

#include <algorithm>
#include <array>
#include <cstdint>

#include <reusable_synth/utils/noncopyable.hpp>

template<int pinChangeCount>
class PinChangeBase : Noncopyable
{
private:
    /** Helper struct to contain <pin, PinChangeBase*> pairing. */
    struct RegisteredPin;

public:
    /**
     * @brief Construct a new Pin Change Base object.
     *
     * @remarks Attaching the instance to an interrupt can fail without warning.
     * Must check the number of instances increased using getUsedCount() or
     * isUsed().
     *
     * @param pin
     */
    PinChangeBase(uint16_t pin) { registerPin(pin); }

    /**
     * @brief Destroy the Pin Change Base object
     *
     */
    virtual ~PinChangeBase() { deregisterPin(this); }

    /**
     * @brief Calls the callback function associated with the pin.
     *
     * Use this function in interrupts.
     *
     * @param pin The pin associated with the pin change event.
     */
    static void irqDispatch(uint16_t pin)
    {
        auto it = std::find_if(instances.begin(),
                               instances.end(),
                               [pin](RegisteredPin registeredPin) {
                                   return registeredPin.pin == pin;
                               });

        if (it == instances.end()) {
            return;
        }

        it->callback();
    }

    /**
     * @brief Get the number of used instances of the class.
     *
     * @return int
     */
    static int getUsedCount()
    {
        int count = 0;
        for (const auto instance : instances) {
            if (instance.ptr != nullptr) {
                count++;
            }
        }
        return count;
    }

    /**
     * @brief Checks if the class was successfully registered to be called by
     * dispatch().
     *
     * @return true Registration of class was successful.
     * @return false Registration of class failed due to insufficient space
     * or already used pin.
     */
    bool isUsed()
    {
        auto it = std::find_if(instances.begin(),
                               instances.end(),
                               [this](RegisteredPin registeredPin) {
                                   return registeredPin.ptr == this;
                               });
        return it != instances.end();
    }

protected:
    virtual void callback() = 0;

private:
    struct RegisteredPin
    {
        uint16_t pin = UINT16_MAX;
        PinChangeBase* ptr = nullptr;
    };

    /**
     * @brief Registers each instance of a class derived from PinChangeBase.
     *
     * @param pin The GPIO pin associated with the interrupt.
     */
    void registerPin(uint16_t pin)
    {
        auto it = std::find_if(instances.begin(),
                               instances.end(),
                               [pin](RegisteredPin registeredPin) {
                                   return registeredPin.pin == pin;
                               });

        // Return early if this pin is already registered.
        if (it != instances.end()) {
            return;
        }

        it = std::find_if(
          instances.begin(), instances.end(), [](RegisteredPin registeredPin) {
              return registeredPin.ptr == nullptr;
          });

        // Return early if no available space in array
        if (it == instances.end()) {
            return;
        }
        it->pin = pin;
        it->ptr = this;
    }

    /**
     * @brief Removes instance of class from registered list.
     *
     * @param ptr Pointer to the instance to remove.
     */
    void deregisterPin(PinChangeBase* ptr)
    {
        auto it = std::find_if(instances.begin(),
                               instances.end(),
                               [this](RegisteredPin registeredPin) {
                                   return registeredPin.ptr == this;
                               });
        if (it != instances.end()) {
            it->pin = UINT16_MAX;
            it->ptr = nullptr;
        }
    }

    /** Array of registered pins and the associated class instances. */
    static inline std::array<RegisteredPin, pinChangeCount> instances = {};
};
