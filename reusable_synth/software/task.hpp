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

/**
 * @brief This contains abstract interface for tasks and task management
 * interfaces.
 *
 */
template<typename TickType = uint32_t>
class timer
{
public:
    using tick_type = TickType;
    using tick_func_type = tick_type (*)();

    timer(tick_func_type get_tick_func)
      : get_tick(get_tick_func)
      , start_tick(0)
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
    tick_func_type get_tick;
    tick_type start_tick;
    tick_type interval_ticks;
};

template<typename TickType = uint32_t>
class task_control_block
{
public:
    using timer_type = timer<TickType>;
    using tick_type = timer_type::tick_type;
    using tick_func_type = tick_type (*)();
    using function_type = void (*)();

    task_control_block(const function_type f,
                       const tick_func_type t,
                       const tick_type c,
                       const tick_type o = 0)
      : function(f)
      , time(timer_type(t))
      , cycle(c)
      , offset(o)
    {
        time.start_interval(o);
    }

    bool execute();

private:
    const function_type function; // function to execute
    timer_type time;              // timer to track next run
    const tick_type cycle;        // how often to run in ticks
    const tick_type offset;       // offset before first run
};

template<std::size_t N, typename TickType = uint32_t>
void scheduler(std::array<task_control_block<TickType>, N>& task_list)
{
    for (;;) {
        // Find first ready task using std::find_if
        std::find_if(task_list.begin(),
                     task_list.end(),
                     [](task_control_block<TickType>& tcb) -> bool {
                         return tcb.execute();
                     });
    }
}

#endif // TASK_HPP