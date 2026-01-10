
#pragma once

/**
 * @brief A base class to prohibit copying (via assignment or construction).
 *
 * From "Real Time C++" by Christopher Kormanyos, which bases it's example on
 * Boost Noncopyable.
 * https://github.com/ckormanyos/real-time-cpp/blob/master/ref_app/src/util/utility/util_noncopyable.h
 *
 * Distributed under the Boost Software License,
 * Version 1.0. (See accompanying file LICENSE_1_0.txt
 * or copy at http://www.boost.org/LICENSE_1_0.txt)
 */
class Noncopyable
{
protected:
    Noncopyable() {}
    ~Noncopyable() {}

private:
    Noncopyable(const Noncopyable&) = delete;
    Noncopyable(Noncopyable&&) = delete;

    auto operator=(const Noncopyable&) -> Noncopyable& = delete;
    auto operator=(Noncopyable&&) -> Noncopyable& = delete;
};
