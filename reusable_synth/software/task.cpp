#include "task.hpp"

extern "C"
{
    volatile uint32_t g_system_tick_count = 0;
}

template<typename TickType>
void timer<TickType>::start_interval(tick_type interval)
{
    start_tick =
      static_cast<TickType>(g_system_tick_count); // set start tick to "now"
    interval_ticks = interval; // set interval "n ticks from now"
}

template<typename TickType>
bool timer<TickType>::timeout() const
{
    const tick_type current = static_cast<TickType>(g_system_tick_count);
    const tick_type elapsed = current - start_tick;
    return elapsed >= interval_ticks;
}

bool task_control_block::execute()
{
    if (time.timeout()) {
        time.start_interval(cycle); // restart timer for 'cycle' ticks from now
        function();

        return true;
    }
    return false;
}

template class timer<uint32_t>;

template<std::size_t N>                                                                                                  
void scheduler(const std::array<task_control_block, N>& task_list)
{
    for (;;) {
        // Find first ready task using std::find_if
        std::find_if(
          task_list.begin(),
          task_list.end(),
          [](task_control_block& tcb) -> bool { return tcb.execute(); });
    }
}
