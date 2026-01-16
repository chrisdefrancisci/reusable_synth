/**
 * @file task.hpp
 * @author ben (bengg@mac.com)
 * @brief Inspired by chapter 15 of Real Time C++
 * @date 2026-01-01
 */

#pragma once

// Standard library includes
#include <algorithm>
#include <array>
#include <chrono>
#include <cstdint>
#include <type_traits>

namespace detail {
template<class>
inline constexpr bool is_duration_v = false;
template<class Rep, class Period>
inline constexpr bool is_duration_v<std::chrono::duration<Rep, Period>> = true;
}

/**
 * @brief This contains abstract interface for tasks and task management
 * interfaces.
 *
 */
template<typename TickType,
         class = std::enable_if_t<detail::is_duration_v<TickType>>>
class Timer
{
public:
    /** */
    using TickFuncType = TickType (*)();

    /**
     * @brief Construct a new Timer object
     *
     * @param getTickFunc
     */
    Timer(TickFuncType getTickFunc)
      : getTick(getTickFunc)
      , startTick(0)
      , intervalTicks(0)
    {
    }

    /**
     * @brief
     *
     * @param interval
     */
    void startInterval(TickType interval)
    {
        startTick = getTick();    // set start tick to "now"
        intervalTicks = interval; // set interval "n ticks from now"
    }

    /**
     * @brief
     *
     * @return true
     * @return false
     */
    bool timeout() const
    {
        const TickType current = getTick();
        const TickType elapsed = current - startTick;
        return elapsed >= intervalTicks;
    }

private:
    TickFuncType getTick;
    TickType startTick;
    TickType intervalTicks;
};

/**
 * @brief
 *
 * @tparam TickType
 */
template<typename TickType = uint32_t>
class TaskControlBlock
{
public:
    /** */
    using TimerType = Timer<TickType>;
    /** */
    using TickFuncType = TickType (*)();
    /** */
    using TaskFuncType = void (*)();

    /**
     * @brief Construct a new Task Control Block object
     *
     * @param task
     * @param getTick
     * @param period
     * @param offset
     */
    TaskControlBlock(const TaskFuncType task,
                     const TickFuncType getTick,
                     const TickType period,
                     const TickType offset = 0)
      : function(task)
      , time(TimerType(getTick))
      , period(period)
      , offset(offset)
    {
        time.startInterval(offset);
    }

    /**
     * @brief
     *
     * @return true
     * @return false
     */
    bool execute()
    {
        if (time.timeout()) {
            time.startInterval(
              period); // restart timer for 'cycle' ticks from now
            function();

            return true;
        }
        return false;
    }

private:
    const TaskFuncType function; // function to execute
    TimerType time;              // timer to track next run
    const TickType period;       // how often to run in ticks
    const TickType offset;       // offset before first run
};

/**
 * @brief
 *
 * @tparam N
 * @tparam TickType
 * @param taskList
 */
template<std::size_t N, typename TickType = uint32_t>
void scheduler(std::array<TaskControlBlock<TickType>, N>& taskList)
{
    for (;;) {
        // Find first ready task using std::find_if
        std::find_if(taskList.begin(),
                     taskList.end(),
                     [](TaskControlBlock<TickType>& tcb) -> bool {
                         return tcb.execute();
                     });
    }
}
