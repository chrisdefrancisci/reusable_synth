#pragma once

#include <optional>

template<typename T>
inline T setFreeFunctionValue(std::optional<T> newVal)
{
    static T val = T();
    if (newVal.has_value()) {
        val = newVal.value();
    }
    return val;
}

template<typename T>
inline T getFreeFunctionValue()
{
    return setFreeFunctionValue<T>(std::nullopt);
}