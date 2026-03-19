#include <gtest/gtest.h>

#include <reusable_synth/software/task.hpp>
#include <tests/stubs/stub_getter.hpp>

using millis = std::chrono::duration<uint32_t, std::milli>;
using millis = std::chrono::duration<uint32_t, std::milli>;
static millis (*getTime)() = getFreeFunctionValue<millis>;
static millis (*setTime)(std::optional<millis>) = setFreeFunctionValue<millis>;
// Mock system tick for testing
// static millis mock_tick_count = millis(0);

// Mock tick source function
// millis mock_get_tick() { return mock_tick_count; }

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
    ASSERT_EQ(setTime(millis(0)), millis(0));

    // Create task that runs every 100ms with tick source
    TaskControlBlock<millis, void (*)()> task(
      test_task, getTime, millis(100), millis(0));

    // Should run immediately (offset = 0)
    EXPECT_TRUE(task.execute());
    EXPECT_EQ(task_run_count, 1);

    // Should not run again immediately
    EXPECT_FALSE(task.execute());
    EXPECT_EQ(task_run_count, 1);

    // Advance time by 50ms - still not ready
    ASSERT_EQ(setTime(millis(50)), millis(50));
    EXPECT_FALSE(task.execute());
    EXPECT_EQ(task_run_count, 1);

    // Advance time to 100ms - should run now
    ASSERT_EQ(setTime(millis(100)), millis(100));
    EXPECT_TRUE(task.execute());
    EXPECT_EQ(task_run_count, 2);
}

TEST(TaskTest, TaskWithOffset)
{
    task_run_count = 0;
    ASSERT_EQ(setTime(millis(0)), millis(0));

    // Create task with 50ms offset
    TaskControlBlock<millis, void (*)()> task(
      test_task, getTime, millis(100), millis(50));

    // Should NOT run immediately (has offset)
    EXPECT_FALSE(task.execute());
    EXPECT_EQ(task_run_count, 0);

    // Advance to offset time
    ASSERT_EQ(setTime(millis(50)), millis(50));
    EXPECT_TRUE(task.execute());
    EXPECT_EQ(task_run_count, 1);
}

// TODO: more varieties of tasks - member functions, lambdas, etc.
TEST(TaskTest, LambdaTest)
{
    int local_task_run_count = 0;
    ASSERT_EQ(setTime(millis(0)), millis(0));

    auto lambda = [&local_task_run_count]() -> void { local_task_run_count++; };

    // Create task with 50ms offset
    TaskControlBlock<millis, decltype(lambda)> task(
      lambda, getTime, millis(100), millis(50));

    // Should NOT run immediately (has offset)
    EXPECT_FALSE(task.execute());
    EXPECT_EQ(local_task_run_count, 0);

    // Advance to offset time
    ASSERT_EQ(setTime(millis(50)), millis(50));
    EXPECT_TRUE(task.execute());
    EXPECT_EQ(local_task_run_count, 1);
}
// TODO: test for scheduler
