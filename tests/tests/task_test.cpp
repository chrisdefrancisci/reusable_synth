#include <gtest/gtest.h>
#include <reusable_synth/software/task.hpp>

// Mock system tick for testing
static volatile uint32_t mock_tick_count = 0;

// Mock tick source function
uint32_t mock_get_tick() {
    return mock_tick_count;
}

// Test task that counts how many times it runs
static int task_run_count = 0;

void test_task() { task_run_count++; }

TEST(TaskTest, TaskExecutesAfterInterval) {
  // Reset test state
  task_run_count = 0;
  mock_tick_count = 0;

  // Create task that runs every 100ms with tick source
  task_control_block<uint32_t> task(
      test_task,
      mock_get_tick,
      timer<uint32_t>::milliseconds(100),
      timer<uint32_t>::milliseconds(0));

  // Should run immediately (offset = 0)
  EXPECT_TRUE(task.execute());
  EXPECT_EQ(task_run_count, 1);

  // Should not run again immediately
  EXPECT_FALSE(task.execute());
  EXPECT_EQ(task_run_count, 1);

  // Advance time by 50ms - still not ready
  mock_tick_count = 50;
  EXPECT_FALSE(task.execute());
  EXPECT_EQ(task_run_count, 1);

  // Advance time to 100ms - should run now
  mock_tick_count = 100;
  EXPECT_TRUE(task.execute());
  EXPECT_EQ(task_run_count, 2);
}

TEST(TaskTest, TaskWithOffset) {
  task_run_count = 0;
  mock_tick_count = 0;

  // Create task with 50ms offset
  task_control_block<uint32_t> task(
      test_task,
      mock_get_tick,
      timer<uint32_t>::milliseconds(100),
      timer<uint32_t>::milliseconds(50));

  // Should NOT run immediately (has offset)
  EXPECT_FALSE(task.execute());
  EXPECT_EQ(task_run_count, 0);

  // Advance to offset time
  mock_tick_count = 50;
  EXPECT_TRUE(task.execute());
  EXPECT_EQ(task_run_count, 1);
}
