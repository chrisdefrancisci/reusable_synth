#include "task.hpp"

template<typename TickType>
void timer<TickType>::start_interval(tick_type interval)
{
    start_tick = static_cast<TickType>(get_tick()); // set start tick to "now"
    interval_ticks = interval; // set interval "n ticks from now"
}

template<typename TickType>
bool timer<TickType>::timeout() const
{
    const tick_type current = static_cast<TickType>(get_tick());
    const tick_type elapsed = current - start_tick;
    return elapsed >= interval_ticks;
}

template<typename TickType>
bool task_control_block<TickType>::execute()
{
    if (time.timeout()) {
        time.start_interval(cycle); // restart timer for 'cycle' ticks from now
        function();

        return true;
    }
    return false;
}

template class timer<uint32_t>;

template class task_control_block<uint32_t>;
