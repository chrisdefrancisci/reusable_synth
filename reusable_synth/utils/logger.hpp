/**
 * @file logger.hpp
 * @author Chris DeFrancisci (chrisdefrancisci@gmail.com)
 * @brief Implements a fixed buffer size logging class.
 * @date 2026-01-02
 */

#include <cstring>
#include <iostream>
#include <source_location>
#include <sstream>
#include <string_view>

#include <reusable_synth/utils/ring_buffer.hpp>

/**
 * @brief Enumerates log severity
 *
 */
enum class LogType
{
    INFO,
    WARNING,
    ERROR
};

/**
 * @brief Holds a string with a LogType enum.
 *
 * @tparam logLen The number of characters (including terminator) a log holds.
 */
template<int logLen>
class Log
{
public:
    Log()
      : logType(LogType::INFO)
    {
        memset(buff, '\0', logLen);
    }

    Log(LogType type, const char* message)
      : logType(type)
    {
        if (strlen(message) < logLen) {
            strcpy(buff, message);
        } else {
            memcpy(buff, message, logLen - 1);
            buff[logLen - 1] = '\0';
        }
    }

    Log(const Log& other)
      : logType(other.logType)
    {
        memcpy(buff, other.buff, logLen);
    }

    Log(Log&& other)
      : logType(other.logType)
    {
        memcpy(buff, other.buff, logLen);
    }

    Log& operator=(const Log& other)
    {
        logType = other.logType;
        memcpy(buff, other.buff, logLen);
        return *this;
    }

    Log& operator=(const Log&& other)
    {
        logType = other.logType;
        memcpy(buff, other.buff, logLen);
        return *this;
    }

    const char* pBuffer() const { return buff; };
    LogType type() const { return logType; };

private:
    char buff[logLen];
    LogType logType;
};

/**
 * @brief A class to store the most recent logs in a ring buffer.
 *
 * @tparam nLogs The number of logs to hold.
 * @tparam logLen The length of each log.
 */
template<int nLogs, int logLen>
class Logger
{
public:
    Logger()
      : fullFlag(false)
    {
    }

    /**
     * @brief Convenience function to add a info message to the logger.
     *
     * @param message The log message.
     * @param location The location associated with the message. Default reports
     * caller location.
     */
    void info(
      const std::string_view message,
      const std::source_location location = std::source_location::current())
    {
        log(LogType::INFO, message, location);
    }

    /**
     * @brief Convenience function to add a warning to the logger.
     *
     * @param message The log message.
     * @param location The location associated with the message. Default reports
     * caller location.
     */
    void warn(
      const std::string_view message,
      const std::source_location location = std::source_location::current())
    {
        log(LogType::WARNING, message, location);
    }

    /**
     * @brief Convenience function to add an error to the logger.
     *
     * @param message The log message.
     * @param location The location associated with the message. Default reports
     * caller location.
     */
    void error(
      const std::string_view message,
      const std::source_location location = std::source_location::current())
    {
        log(LogType::ERROR, message, location);
    }

    /**
     * @brief Adds a log to the logger.
     *
     * @remarks If the buffer is full, a warning log will be added.
     *
     * @param type The log severity.
     * @param message The log message.
     * @param location The location associated with the message. Default reports
     * caller location.
     */
    void log(
      LogType type,
      const std::string_view message,
      const std::source_location location = std::source_location::current())
    {
        // TODO: find alternative solution, stringstream is likely a terrible
        // choice for a memory constrained system
        std::stringstream ss;
        if (logsBuffer.full() && !fullFlag) {
            fullFlag = true;
            ss << std::source_location::current().file_name() << ":"
               << std::source_location::current().line() << " "
               << "Logger overflow!";

            logsBuffer.push_back(
              Log<logLen>(LogType::WARNING, ss.str().c_str()));
            ss.clear();
            ss.str("");
        }
        ss << location.file_name() << ":" << location.line() << " " << message;

        logsBuffer.push_back(Log<logLen>(type, ss.str().c_str()));
    }

    /**
     * @brief Removes the oldest log from the logger.
     *
     * @return std::optional<Log<logLen>> The removed log if there was one or
     * std::nullopt.
     */
    std::optional<Log<logLen>> remove_log()
    {
        fullFlag = false;
        return logsBuffer.pop_front();
    }

private:
    RingBuffer<Log<logLen>, nLogs> logsBuffer;
    bool fullFlag;
};
