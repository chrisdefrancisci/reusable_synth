#include <gtest/gtest.h>
#include "task.hpp"

// Mock system tick for testing
extern "C"
{
    extern volatile uint32_t g_system_tick_count;
}

// Test task that counts how many times it runs
static int task_run_count = 0;

void test_task()
{
    task_run_count++;
}

TEST(TaskTest, TaskExecutesAfterInterval)
{
    // Reset test state
    task_run_count = 0;
    g_system_tick_count = 0;

    // Create task that runs every 100ms
    task_control_block task(test_task,
                            task_control_block::timer_type::milliseconds(100));

    // Should run immediately (offset = 0)
    EXPECT_TRUE(task.execute());
    EXPECT_EQ(task_run_count, 1);

    // Should not run again immediately
    EXPECT_FALSE(task.execute());
    EXPECT_EQ(task_run_count, 1);

    // Advance time by 50ms - still not ready
    g_system_tick_count = 50;
    EXPECT_FALSE(task.execute());
    EXPECT_EQ(task_run_count, 1);

    // Advance time to 100ms - should run now
    g_system_tick_count = 100;
    EXPECT_TRUE(task.execute());
    EXPECT_EQ(task_run_count, 2);
}

TEST(TaskTest, TaskWithOffset)
{
    task_run_count = 0;
    g_system_tick_count = 0;

    // Create task with 50ms offset
    task_control_block task(test_task,
                            task_control_block::timer_type::milliseconds(100),
                            task_control_block::timer_type::milliseconds(50));

    // Should NOT run immediately (has offset)
    EXPECT_FALSE(task.execute());
    EXPECT_EQ(task_run_count, 0);

    // Advance to offset time
    g_system_tick_count = 50;
    EXPECT_TRUE(task.execute());
    EXPECT_EQ(task_run_count, 1);
}
