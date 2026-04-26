#include <gtest/gtest.h>

#include <reusable_synth/software/task.hpp>
#include <tests/stubs/stub_getter.hpp>

using Millis = std::chrono::duration<uint32_t, std::milli>;
static Millis (*getTime)() = get_free_function_value<Millis>;
static Millis (*setTime)(std::optional<Millis>) =
  set_free_function_value<Millis>;
// Mock system tick for testing
// static millis mock_tick_count = millis(0);

// Mock tick source function
// millis mock_get_tick() { return mock_tick_count; }

// Test task that counts how many times it runs
static int staticTaskRunCount = 0;

void test_task()
{
    staticTaskRunCount++;
}

class MockFunctor
{
public:
    void operator()() { count++; }
    [[nodiscard]] auto getCount() const -> int { return count; }

private:
    int count = 0;
};

TEST(TaskTest, TaskExecutesAfterInterval)
{
    // Reset test state
    staticTaskRunCount = 0;
    ASSERT_EQ(setTime(Millis(0)), Millis(0));

    // Create task that runs every 100ms with tick source
    TaskControlBlock<Millis, void (*)()> task(
      test_task, getTime, Millis(100), Millis(0));

    // Should run immediately (offset = 0)
    EXPECT_TRUE(task.execute());
    EXPECT_EQ(staticTaskRunCount, 1);

    // Should not run again immediately
    EXPECT_FALSE(task.execute());
    EXPECT_EQ(staticTaskRunCount, 1);

    // Advance time by 50ms - still not ready
    ASSERT_EQ(setTime(Millis(50)), Millis(50));
    EXPECT_FALSE(task.execute());
    EXPECT_EQ(staticTaskRunCount, 1);

    // Advance time to 100ms - should run now
    ASSERT_EQ(setTime(Millis(100)), Millis(100));
    EXPECT_TRUE(task.execute());
    EXPECT_EQ(staticTaskRunCount, 2);
}

TEST(TaskTest, TaskWithOffset)
{
    staticTaskRunCount = 0;
    ASSERT_EQ(setTime(Millis(0)), Millis(0));

    // Create task with 50ms offset
    TaskControlBlock<Millis, void (*)()> task(
      test_task, getTime, Millis(100), Millis(50));

    // Should NOT run immediately (has offset)
    EXPECT_FALSE(task.execute());
    EXPECT_EQ(staticTaskRunCount, 0);

    // Advance to offset time
    ASSERT_EQ(setTime(Millis(50)), Millis(50));
    EXPECT_TRUE(task.execute());
    EXPECT_EQ(staticTaskRunCount, 1);
}

TEST(TaskTest, LambdaTest)
{
    int localTaskRunCount = 0;
    ASSERT_EQ(setTime(Millis(0)), Millis(0));

    auto lambda = [&localTaskRunCount]() -> void { localTaskRunCount++; };

    // Create task with 50ms offset
    TaskControlBlock<Millis, decltype(lambda)> task(
      lambda, getTime, Millis(100), Millis(50));

    // Should NOT run immediately (has offset)
    EXPECT_FALSE(task.execute());
    EXPECT_EQ(localTaskRunCount, 0);

    // Advance to offset time
    ASSERT_EQ(setTime(Millis(50)), Millis(50));
    EXPECT_TRUE(task.execute());
    EXPECT_EQ(localTaskRunCount, 1);
}

TEST(TaskTest, FunctorTest)
{
    ASSERT_EQ(setTime(Millis(0)), Millis(0));

    MockFunctor func;

    // Create task that runs every 100ms with tick source
    TaskControlBlock<Millis, MockFunctor> task(
      func, getTime, Millis(100), Millis(0));

    // Should run immediately (offset = 0)
    EXPECT_TRUE(task.execute());
    EXPECT_EQ(func.getCount(), 1);

    // Advance time to 100ms - should run now
    ASSERT_EQ(setTime(Millis(100)), Millis(100));
    EXPECT_TRUE(task.execute());
    EXPECT_EQ(func.getCount(), 2);
}
