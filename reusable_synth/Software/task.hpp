/**
 * @file task.hpp
 * @author ben (bengg@mac.com)
 * @brief
 * @date 2026-01-01
 */
#ifndef TASK_HPP
#define TASK_HPP

// Standard library includes
#include <algorithm>
#include <array>
#include <cstdint>
#include <utility>

/**
 * @brief This contains abstract interface for tasks and task management
 * interfaces.
 *
 *
 */

void
scheduler();

template<typename TickType = uint32_t>
class timer
{
public:
    using tick_type = TickType;

    timer()
      : start_tick(0)
      , interval_ticks(0)
    {
    }

    void start_interval(tick_type interval);
    bool timeout() const;

    static constexpr tick_type milliseconds(tick_type ms)
    {
        return ms;
    } // assuming 1 tick = 1 ms
    static constexpr tick_type seconds(tick_type s) { return s * 1000; }

private:
    tick_type start_tick;
    tick_type interval_ticks;
};

class task_control_block
{
public:
    using timer_type = timer<uint32_t>;
    using tick_type = timer_type::tick_type;
    using function_type = void (*)();

    task_control_block(const function_type f,
                       const tick_type c,
                       const tick_type o = 0U)
      : function(f)
      , cycle(c)
      , time()
    {
        time.start_interval(o);
    }

    bool execute();

private:
    const function_type function;
    const tick_type cycle; // how often to run in ticks
    timer_type time;       // timer to track next run
};

#endif // TASK_HPP