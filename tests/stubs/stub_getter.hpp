#pragma once

#include <optional>

template<typename T>
inline auto set_free_function_value(std::optional<T> newVal) -> T
{
    static T val = T();
    if (newVal.has_value()) {
        val = newVal.value();
    }
    return val;
}

template<typename T>
inline auto get_free_function_value() -> T
{
    return set_free_function_value<T>(std::nullopt);
}