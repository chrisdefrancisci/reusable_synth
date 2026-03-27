/**
 * @file ring_buffer.hpp
 * @author Chris DeFrancisci (chrisdefrancisci@gmail.com)
 * @brief
 * @date 2026-01-02
 */

#pragma once

#include <array>
#include <cstddef>
#include <iostream>
#include <optional>

#include <reusable_synth/utils/noncopyable.hpp>

/**
 * @brief A fixed-size ring buffer (circular buffer) class.
 *
 * This buffer implements a FIFO (First In First Out) queue occupying
 * a fixed size in memory.
 *
 * @tparam T The value type.
 * @tparam N The number of elements.
 */
template<typename T, const size_t N>
class RingBuffer : private Noncopyable
{
public:
    RingBuffer()
      : back(buffer.data())
      , front(buffer.data())
    {
    }

    /**
     * @brief Adds a value to the back of the buffer.
     *
     * @remark If the buffer is full, this will overwrite without warning.
     *
     * @param inValue The new value.
     */
    void push_back(const T& inValue)
    {
        if (full()) {
            (void)pop_front(); // explicitly ignoring return value
        }
        *back = inValue;
        safe_increment(back);
        count++;
    };

    /**
     * @brief Removes value from the front of the buffer.
     *
     * @return std::optional<T> Front value of buffer or std::nullopt.
     */
    auto pop_front() -> std::optional<T>
    {
        std::optional<T> ret;
        if (empty()) {
            ret = std::nullopt;
        } else {
            count--;
            ret = *front;
            safe_increment(front);
        }
        return ret;
    };

    /**
     * @brief Returns true if there are no values in the buffer, false
     * otherwise.
     */
    auto empty() -> bool { return count == 0; }

    /**
     * @brief Returns true if all spots in the buffer are filled, false
     * otherwise.
     */
    auto full() -> bool { return count == N; }

    /**
     * @brief Returns the number of elements currently in the buffer.
     */
    auto size() -> size_t { return count; }

private:
    void safe_increment(T*& ptr)
    {
        if (ptr == &buffer[N - 1]) {
            ptr = buffer.data();
        } else {
            ptr++; // NOLINT(cppcoreguidelines-pro-bounds-pointer-arithmetic)
        }
    }

    std::array<T, N> buffer{};
    T* back;
    T* front;
    size_t count{};
};
