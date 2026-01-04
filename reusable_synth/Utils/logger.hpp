/**
 * @file logger.hpp
 * @author Chris DeFrancisci (chrisdefrancisci@gmail.com)
 * @brief Implements a fixed buffer size logging class.
 * @date 2026-01-02
 */

#include <cstring>
#include <iostream>

#include <reusable_synth/Utils/ring_buffer.hpp>

template<int nLogs, int logLen>
class Logger
{
public:
    Logger();

    void aadf();

private:
    class Log
    {
    public:
        const char* pBuffer() { return buff; };

    private:
        char buff[logLen] = { 0 };
    };
    RingBuffer<Log, nLogs> logs;
};
