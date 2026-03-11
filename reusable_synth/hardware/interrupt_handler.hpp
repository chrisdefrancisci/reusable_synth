/**
 * @file interrupt_handler.hpp
 * @author Chris DeFrancisci (chrisdefrancisci@gmail.com)
 * @brief From
 * https://www.reddit.com/r/embedded/comments/way0ta/how_can_i_write_effective_isr_handlers_in_c_code/
 * @date 2026-01-26
 */

/**
 * @brief Defines an (object, function) wrapper that can be used to call class
 * methods in an interrupt.
 *
 * InterruptHandler objects can be defined statically in a file with the C
 * interrupt.
 * @todo usage example
 */

#pragma once

class InterruptHandler
{
public:
    /**
     * @brief Connects a class method as a callback.
     *
     * @tparam Func The address of the method.
     * @tparam Class The class type.
     * @param obj An object of type Class.
     */
    template<auto Func, typename Class>
    void connect(Class* obj)
    {
        callback.obj = obj;
        callback.func = [](void* data) {
            Class* obj = static_cast<Class*>(data);
            (obj->*Func)();
        };
    }

    /**
     * @brief Connects a free function as a callback.
     *
     * @tparam (*Func)() The function to call.
     */
    template<void (*Func)()>
    void connect()
    {
        callback.obj = nullptr;
        callback.func = [](void* data) { Func(); };
    }

    /**
     * @brief Returns object to its original, pre-connect state.
     */
    void disconnect()
    {
        callback.obj = nullptr;
        callback.func = nullptr;
    }

    /**
     * @brief Calls the callback.
     */
    inline void operator()() const
    {
        if (callback.func) {
            callback.func(callback.obj);
        }
    }

    /**
     * @brief Checks if any function is connected to this.
     *
     * @return true If func != nullptr.
     * @return false If func == nullptr.
     */
    bool isConnected() const { return callback.func != nullptr; }

private:
    struct Callback
    {
        using Func = void (*)(void*);
        void* obj = nullptr;
        Func func = nullptr;
    };
    Callback callback;
};