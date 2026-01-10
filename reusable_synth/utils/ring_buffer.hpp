/**
 * @file ring_buffer.hpp
 * @author Chris DeFrancisci (chrisdefrancisci@gmail.com)
 * @brief
 * @date 2026-01-02
 */

#pragma once

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
      : buffer{}
      , back{ buffer }
      , front{ buffer }
      , count{ 0 } {};

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
    std::optional<T> pop_front()
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
    bool empty() { return count == 0; }

    /**
     * @brief Returns true if all spots in the buffer are filled, false
     * otherwise.
     */
    bool full() { return count == N; }

    /**
     * @brief Returns the number of elements currently in the buffer.
     */
    size_t size() { return count; }

private:
    inline void safe_increment(T*& ptr)
    {
        if (ptr == &buffer[N - 1]) {
            ptr = &buffer[0];
        } else {
            ptr++;
        }
    }

    T buffer[N];
    T* back;
    T* front;
    size_t count;
};
