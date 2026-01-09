/**
 * @file ring_buffer.hpp
 * @author Chris DeFrancisci (chrisdefrancisci@gmail.com)
 * @brief
 * @date 2026-01-02
 */

#pragma once

#include <cstddef>
#include <reusable_synth/Utils/noncopyable.hpp>

template<typename T, const size_t N>
class RingBuffer : private Noncopyable
{
public:
    RingBuffer() : in(0), out(0) {}

    bool isEmpty() const
    {
        return in == out;
    }

    bool isFull() const
    {
        return (in + 1) % N == out;
    }

    T read()
    {
        if (isEmpty()) {
            throw std::runtime_error("RingBuffer is empty");
        }
        T value = buffer[out];
        out = (out + 1) % N;
        return value;
    }

    void write(const T& value)
    {
        buffer[in] = value;
        in = (in + 1) % N;
        if (in == out) {
            // Buffer overflow, advance out to overwrite oldest data
            out = (out + 1) % N;
        }
    }

private:
    T buffer[N];
    size_t in;
    size_t out;
};
