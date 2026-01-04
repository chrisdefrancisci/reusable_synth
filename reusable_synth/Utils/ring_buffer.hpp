/**
 * @file ring_buffer.hpp
 * @author Chris DeFrancisci (chrisdefrancisci@gmail.com)
 * @brief
 * @date 2026-01-02
 */

#pragma once

#include <cstddef>
#include <reusable_synth/Utils/noncopyable.hpp>

#include "noncopyable.hpp"

template<typename T, const size_t N>
class RingBuffer : private Noncopyable
{
public:
    RingBuffer() {

    };

private:
    T buffer[N];
    T* in;
    T* out;
};
