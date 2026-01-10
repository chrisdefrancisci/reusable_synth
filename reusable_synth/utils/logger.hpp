/**
 * @file logger.hpp
 * @author Chris DeFrancisci (chrisdefrancisci@gmail.com)
 * @brief Implements a fixed buffer size logging class.
 * @date 2026-01-02
 */

#include <cstring>
#include <iostream>
#include <string_view>

#include <reusable_synth/utils/ring_buffer.hpp>

template<int nLogs, int logLen>
class Logger {
public:
    Logger();

    void aadf();

private:
    class Log {
    public:
        Log()
          : type(LogType::INFO) {
            memset(buff, '\0', logLen);
        };
        const char* pBuffer() { return buff; };

    private:
        enum class LogType { INFO, WARNING, ERROR };
        char buff[logLen] = { 0 };
        LogType type;
    };

    RingBuffer<Log, nLogs> logs;
};
