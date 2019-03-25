#pragma once

#include <memory>
#include <string>
#include <Windows.h>

#define LOG_PATH "log.txt"

class Logger
{
public:
    static void init();
    ~Logger();

    template<typename ... Args>
    static void LOG_INFO(const std::string& format, Args&& ...args);

    template<typename ... Args>
    static void LOG_WARNING(const std::string& format, Args&& ... args);

    template<typename ... Args>
    static void LOG_ERROR(const std::string& format, Args&& ... args);

private:
    // Prints prefix, formatted string, current time to console and file
    static void log(unsigned short color, const std::string prefix, const std::string text);

    template<typename ... Args>
    static std::string formatString(const std::string& format, Args&& ... args);
    static std::string timeToString();

    static HANDLE consoleHandle;

    static std::ofstream logFile;
};

template<typename ... Args>
void Logger::LOG_INFO(const std::string& format, Args&& ... args)
{
    std::string text = Logger::formatString(format, args ...);

    Logger::log(FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE | FOREGROUND_INTENSITY, "[INFO]", text);
}

template<typename ... Args>
void Logger::LOG_WARNING(const std::string& format, Args&& ... args)
{
    std::string text = Logger::formatString(format, args ...);

    Logger::log(FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_INTENSITY, "[WARNING]", text);
}

template<typename ... Args>
void Logger::LOG_ERROR(const std::string& format, Args&& ... args)
{
    std::string text = Logger::formatString(format, args ...);

    Logger::log(FOREGROUND_RED | FOREGROUND_INTENSITY, "[ERROR]", text);
}

template<typename ... Args>
std::string Logger::formatString(const std::string& format, Args&& ... args)
{
    // Create a char buffer to hold the formatted string and a null termination
    size_t size = snprintf(nullptr, 0, format.c_str(), std::forward<Args>(args) ...) + 1;
    std::unique_ptr<char[]> buffer(new char[size]);

    // Format string
    snprintf(buffer.get(), size, format.c_str(), std::forward<Args>(args) ...);

    // Convert to std::string without null termination
    return std::string(buffer.get(), buffer.get() + size - 1);
}
