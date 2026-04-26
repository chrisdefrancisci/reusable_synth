/**
 * @file task.hpp
 * @author ben (bengg@mac.com)
 * @brief Inspired by chapter 15 of Real Time C++
 * @date 2026-01-01
 */

#pragma once

// Standard library includes
#include "reusable_synth/utils/noncopyable.hpp"
#include <algorithm>
#include <array>
#include <chrono>
#include <memory>
#include <type_traits>

namespace detail {
/**
 * @brief Determines if type is std::chrono::duration
 *
 * @remarks This is suggested by
 * https://en.cppreference.com/w/cpp/chrono/duration/floor, but it is suggested
 * to not do this by https://en.cppreference.com/w/cpp/types/enable_if.html
 *
 * @tparam class
 */
template<class>
inline constexpr bool is_duration_v = false;

/**
 * @brief Template specialization to determine if type is std::chrono::duration.
 *
 * @tparam Rep underlying type
 * @tparam Period units
 */
template<class Rep, class Period>
inline constexpr bool is_duration_v<std::chrono::duration<Rep, Period>> = true;
} // namespace detail

/**
 * @brief A helper class to evaluate if a period of time has elapsed.
 *
 * @tparam TickType Must be of type std::chrono::duration
 */
template<typename TickType,
         class = std::enable_if_t<detail::is_duration_v<TickType>>>
class Timer
{
public:
    /** The current time must be given by a function (not lambda or object). */
    using TickFuncType = TickType (*)();

    /**
     * @brief Construct a new Timer object
     *
     * @param getTickFunc Callback function returning current time.
     */
    Timer(TickFuncType getTickFunc)
      : getTick(getTickFunc)
      , startTick(0)
      , intervalTicks(0)
    {
    }

    /**
     * @brief Uses the current time so that timeout returns true at current time
     * + interval.
     *
     * @param interval The interval to timeout.
     */
    void startInterval(TickType interval)
    {
        startTick = getTick();    // set start tick to "now"
        intervalTicks = interval; // set interval "n ticks from now"
    }

    /**
     * @brief Evaluates the state of the timer. Does not start a new interval.
     *
     * @return true If interval has elapsed since startInterval.
     * @return false If interval has not yet elapsed since startInterval.
     */
    [[nodiscard]] auto timeout() const -> bool
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
 * @brief Defines interface to control tasks of different types.
 *
 * @tparam TickType The type to use for the timer.
 */
template<typename TickType>
class TaskControlBlockInterface : Noncopyable
{
public:
    virtual auto execute() -> bool = 0;
    virtual ~TaskControlBlockInterface() = default;
};

/**
 * @brief Periodically completes a task using a timer.
 *
 * @tparam TickType The type to use for the timer.
 * @tparam TaskType The callable object to execute.
 */
template<typename TickType, typename Callable>
class TaskControlBlock : public TaskControlBlockInterface<TickType>
{
public:
    /** Helper for timer template. */
    using TimerType = Timer<TickType>;
    /** Supplied tick callback must be acceptable to Timer class. */
    using TickFuncType = typename Timer<TickType>::TickFuncType;

    /**
     * @brief Construct a new Task Control Block object
     *
     * @param task The task to update periodically.
     * @param getTick The current tick callback function.
     * @param period The task evaluation period.
     * @param offset Initial offset from task evaluation, in order to prevent
     * tasks from regularly being scheduled at the same time.
     */
    TaskControlBlock(Callable& task,
                     const TickFuncType getTick,
                     const TickType period,
                     const TickType offset = 0)
      : task(task)
      , time(TimerType(getTick))
      , period(period)
      , offset(offset)
    {
        time.startInterval(offset);
    }

    /**
     * @brief Execute the task if the period has elapsed.
     *
     * @return true If the task executed.
     * @return false If the task did not execute.
     */
    auto execute() -> bool override
    {
        if (time.timeout()) {
            // restart timer for 'cycle' ticks from now
            time.startInterval(period);
            task();
            return true;
        }
        return false;
    }

private:
    Callable& task; // function to execute

    TimerType time;        // timer to track next run
    const TickType period; // how often to run in ticks
    const TickType offset; // offset before first run
};

/**
 * @brief TaskControlBlock specialization for void(*)().
 *
 * @tparam TickType The type to use for the timer.
 */
template<typename TickType>
class TaskControlBlock<TickType, void (*)()>
  : public TaskControlBlockInterface<TickType>
{
public:
    /** Helper for timer template. */
    using TimerType = Timer<TickType>;
    /** Supplied tick callback must be acceptable to Timer class. */
    using TickFuncType = typename Timer<TickType>::TickFuncType;

    /**
     * @brief Construct a new Task Control Block object
     *
     * @param task The task to update periodically.
     * @param getTick The current tick callback function.
     * @param period The task evaluation period.
     * @param offset Initial offset from task evaluation, in order to prevent
     * tasks from regularly being scheduled at the same time.
     */
    TaskControlBlock(void (*task)(),
                     const TickFuncType getTick,
                     const TickType period,
                     const TickType offset = 0)
      : task(task)
      , time(TimerType(getTick))
      , period(period)
      , offset(offset)
    {
        time.startInterval(offset);
    }

    /**
     * @brief Execute the task if the period has elapsed.
     *
     * @return true If the task executed.
     * @return false If the task did not execute.
     */
    auto execute() -> bool override
    {
        if (time.timeout()) {
            // restart timer for 'cycle' ticks from now
            time.startInterval(period);
            task();
            return true;
        }
        return false;
    }

private:
    void (*task)(); // function to execute

    TimerType time;        // timer to track next run
    const TickType period; // how often to run in ticks
    const TickType offset; // offset before first run
};

template<typename TickType>
TaskControlBlock(void (*)(),
                 typename Timer<TickType>::TickFuncType,
                 TickType,
                 TickType) -> TaskControlBlock<TickType, void (*)()>;

/**
 * @brief Runs tasks in infinite loop; does not return.
 *
 * This wraps std::find_if to form a simple priority scheduler - the items
 * first in the supplied array have higher priority. Note that there is no
 * checking if a task is "starved".
 *
 * @todo Replace std::find_if with a loop.
 * @todo Determine if task timing is not met
 *
 * @tparam N The number of tasks.
 * @tparam TickType The timue unit used
 * @param taskList The list of tasks (i.e., callable objects) in order of
 * priority.
 */
template<std::size_t N, typename TickType>
void scheduler(
  std::array<std::unique_ptr<TaskControlBlockInterface<TickType>>, N>& taskList)
{
    for (;;) {
        // Find first ready task using std::find_if.
        // Mark as void because we are not actually interested in the return
        // value.
        (void)std::find_if(
          taskList.begin(),
          taskList.end(),
          [](const std::unique_ptr<TaskControlBlockInterface<TickType>>& tcb)
            -> bool { return tcb->execute(); });
    }
}
