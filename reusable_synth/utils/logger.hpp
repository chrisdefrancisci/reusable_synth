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

// TODO: LogType to string

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
        memset(buff.data(), '\0', logLen);
    }

    Log(LogType type, const char* message)
      : logType(type)
    {
        if (strlen(message) < logLen) {
            strcpy(buff.data(), message);
        } else {
            memcpy(buff.data(), message, logLen - 1);
            buff[logLen - 1] = '\0';
        }
    }

    ~Log() = default;

    Log(const Log& other)
      : logType(other.logType)
    {
        memcpy(buff.data(), other.buff.data(), logLen);
    }

    Log(Log&& other) noexcept
      : logType(other.logType)
    {
        memcpy(buff.data(), other.buff.data(), logLen);
    }

    auto operator=(const Log& other) -> Log&
    {
        if (&other == this) {
            return *this;
        }
        logType = other.logType;
        memcpy(buff.data(), other.buff.data(), logLen);
        return *this;
    }

    auto operator=(Log&& other) noexcept -> Log&
    {
        logType = other.logType;
        memcpy(buff.data(), other.buff.data(), logLen);
        return *this;
    }

    [[nodiscard]] auto pBuffer() const -> const char* { return buff.data(); };
    [[nodiscard]] auto type() const -> LogType { return logType; };

private:
    std::array<char, logLen> buff{};
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
    Logger() = default;

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
        std::stringstream stream;
        if (logsBuffer.full() && !fullFlag) {
            fullFlag = true;
            stream << std::source_location::current().file_name() << ":"
                   << std::source_location::current().line() << " "
                   << "Logger overflow!";

            logsBuffer.push_back(
              Log<logLen>(LogType::WARNING, stream.str().c_str()));
            stream.clear();
            stream.str("");
        }
        stream << location.file_name() << ":" << location.line() << " "
               << message;

        logsBuffer.push_back(Log<logLen>(type, stream.str().c_str()));
    }

    /**
     * @brief Removes the oldest log from the logger.
     *
     * @return std::optional<Log<logLen>> The removed log if there was one or
     * std::nullopt.
     */
    auto remove_log() -> std::optional<Log<logLen>>
    {
        fullFlag = false;
        return logsBuffer.pop_front();
    }

private:
    RingBuffer<Log<logLen>, nLogs> logsBuffer;
    bool fullFlag = false;
};
