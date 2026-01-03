///////////////////////////////////////////////////////////////////////////////
//  Copyright Christopher Kormanyos 2007 - 2023.
//  Distributed under the Boost Software License,
//  Version 1.0. (See accompanying file LICENSE_1_0.txt
//  or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#pragma once


class Noncopyable
{
protected:
    Noncopyable() = default; 
    ~Noncopyable() = default;

private:
    Noncopyable(const Noncopyable&) = delete;
    Noncopyable(Noncopyable&&)      = delete;

    auto operator=(const Noncopyable&) -> Noncopyable& = delete; 
    auto operator=(Noncopyable&&)      -> Noncopyable& = delete; 
};

