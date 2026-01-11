#include <cstring>
#include <source_location>
#include <sstream>
#include <string>

#include <gtest/gtest.h>

#include <reusable_synth/utils/logger.hpp>

TEST(LoggerTest, StringTooLong) {}

TEST(LoggerTest, AddOneString) {
  constexpr int nLogs = 10;
  constexpr int logLen = 200;
  Logger<nLogs, logLen> logger;
  std::stringstream expectedVal;
  expectedVal << std::source_location::current().file_name() << ":"
              << (std::source_location::current().line() + 1) << " test";
  logger.info("test");
  auto val = logger.remove_log();

  ASSERT_TRUE(val.has_value());
  EXPECT_STREQ(val->pBuffer(), expectedVal.str().c_str());
  EXPECT_EQ(val->type(), LogType::INFO);

  //   std::cout << val->pBuffer() << std::endl;
  //   std::cout << expectedVal.str() << std::endl;
}

TEST(LoggerTest, StringOverflow) {
  constexpr int nLogs = 10;
  constexpr int logLen = 200;
  Logger<nLogs, logLen> logger;

  char overflowChars[logLen];
  memset(overflowChars, 'a', logLen);
  overflowChars[logLen - 1] = '\0';

  logger.info(overflowChars);
  std::stringstream expectedVal;
  expectedVal << std::source_location::current().file_name() << ":"
              << (std::source_location::current().line() - 3) << " ";
  auto startLen = strlen(expectedVal.str().c_str());
  expectedVal << std::string(logLen - startLen - 1, 'a');

  auto val = logger.remove_log();

  ASSERT_TRUE(val.has_value());
  EXPECT_STREQ(val->pBuffer(), expectedVal.str().c_str());
  EXPECT_EQ(val->type(), LogType::INFO);

  //   std::cout << val->pBuffer() << std::endl;
  //   std::cout << expectedVal.str() << std::endl;
}

TEST(LoggerTest, LoggerOverflow) {
  constexpr int nLogs = 10;
  constexpr int logLen = 200;
  Logger<nLogs, logLen> logger;
  for (int i = 0; i <= nLogs; i++) {
    logger.info("test");
  }
  // At this point, the second-to-last log should be the warning log
  // And the log after is the log that caused the warning
  for (int i = 0; i < nLogs - 2; i++) {
    auto val = logger.remove_log();
    ASSERT_TRUE(val.has_value());
    EXPECT_EQ(val->type(), LogType::INFO) << "Iteration: " << i;
    // std::cout << val->pBuffer() << std::endl;
  }
  auto val = logger.remove_log();
  ASSERT_TRUE(val.has_value());
  EXPECT_EQ(val->type(), LogType::WARNING) << "First" << std::endl;
  //   std::cout << val->pBuffer() << std::endl;
  val = logger.remove_log();
  ASSERT_TRUE(val.has_value());
  EXPECT_EQ(val->type(), LogType::INFO) << "Second" << std::endl;
  //   std::cout << val->pBuffer() << std::endl;
}