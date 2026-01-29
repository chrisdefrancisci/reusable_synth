/**
 * @file interrupt_handler.hpp
 * @author Chris DeFrancisci (chrisdefrancisci@gmail.com)
 * @brief From
 * https://www.reddit.com/r/embedded/comments/way0ta/comment/ii42ly3/
 * @date 2026-01-26
 */

class InterruptHandler
{
public:
    template<auto Func, typename Class>
    void connect(Class* obj)
    {
        callback.obj = obj;
        callback.func = [](void* data) {
            Class* obj = static_cast<Class*>(data);
            (obj->*Func)();
        };
    }

    template<void (*Func)()>
    void connect()
    {
        callback.obj = nullptr;
        callback.func = [](void* data) { Func(); };
    }

    void disconnect()
    {
        callback.obj = nullptr;
        callback.func = nullptr;
    }

    void operator()() const
    {
        if (callback.func) {
            callback.func(callback.obj);
        }
    }

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