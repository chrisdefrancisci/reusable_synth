#include "task.hpp"

extern "C"
{
    volatile uint32_t g_system_tick_count = 0;
}

template <typename TickType>
void timer<TickType>::start_interval(tick_type interval)
{
    start_tick = static_cast<TickType>(g_system_tick_count); // set start tick to "now"
    interval_ticks = interval;                               // set interval "n ticks from now"
}

template <typename TickType>
bool timer<TickType>::timeout() const
{
    const tick_type current = static_cast<TickType>(g_system_tick_count);
    const tick_type elapsed = current - start_tick;
    return elapsed >= interval_ticks;
}

bool task_control_block::execute()
{
    if (time.timeout())
    {
        time.start_interval(cycle); // restart timer for 'cycle' ticks from now
        function();

        return true;
    }
    return false;
}

template class timer<uint32_t>;

void task_a()
{
    // TODO: Blink
}

void task_b()
{
    // TODO: Read sensor
}

void task_c()
{
    // TODO: Send telemetry
}

std::array<task_control_block, 3> task_list = {task_control_block(task_a,
                                                                  task_control_block::timer_type::milliseconds(2),
                                                                  task_control_block::timer_type::milliseconds(0)),

                                               task_control_block(task_b,
                                                                  task_control_block::timer_type::milliseconds(8),
                                                                  task_control_block::timer_type::milliseconds(7)),

                                               task_control_block(task_c,
                                                                  task_control_block::timer_type::milliseconds(16),
                                                                  task_control_block::timer_type::milliseconds(13))};

void scheduler()
{
    for (;;)
    {
        // Find first ready task using std::find_if
        std::find_if(task_list.begin(),
                     task_list.end(),
                     [](task_control_block &tcb) -> bool
                     {
                         return tcb.execute();
                     });
    }
}