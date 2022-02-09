#pragma once

#include <Engine/Utils/Debug.hpp>

#include <memory>
#include <string>

#ifdef PLATFORM_WINDOWS
    #include <Windows.h>
#endif

#define LOG_PATH "log.txt"

#define INFO_COLOR FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE | FOREGROUND_INTENSITY
#define WARN_COLOR FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_INTENSITY
#define ERROR_COLOR FOREGROUND_RED | FOREGROUND_INTENSITY

#define LOG_INFOF(format, ...) Logger::logInfof(__FILE__, __LINE__, format, __VA_ARGS__)
#define LOG_WARNINGF(format, ...) Logger::logWarningf(__FILE__, __LINE__, format, __VA_ARGS__)
#define LOG_ERRORF(format, ...) Logger::logErrorf(__FILE__, __LINE__, format, __VA_ARGS__)

#define LOG_INFO(message, ...) Logger::logInfo(__FILE__, __LINE__, message)
#define LOG_WARNING(message) Logger::logWarning(__FILE__, __LINE__, message)
#define LOG_ERROR(message) Logger::logError(__FILE__, __LINE__, message)

class Logger
{
public:
    static void init();
    ~Logger();

    template<typename ... Args>
    static void logInfof(const char* pFile, int lineNr, const std::string& format, Args&& ... args);

    template<typename ... Args>
    static void logWarningf(const char* pFile, int lineNr, const std::string& format, Args&& ... args);

    template<typename ... Args>
    static void logErrorf(const char* pFile, int lineNr, const std::string& format, Args&& ... args);

    static void logInfo(const char* pFile, int lineNr, const std::string& message);
    static void logWarning(const char* pFile, int lineNr, const std::string& message);
    static void logError(const char* pFile, int lineNr, const std::string& message);

    template<typename ... Args>
    static std::string formatString(const std::string& format, Args&& ... args);

private:
    // Prints prefix, formatted string, current time to console and file
    static void log(const char* pFile, int lineNr, unsigned short color, const std::string& severity, const std::string& message);
    static std::string timeToString();

    static HANDLE consoleHandle;
    static std::ofstream logFile;
};

template<typename ... Args>
void Logger::logInfof(const char* pFile, int lineNr, const std::string& format, Args&& ... args)
{
    const std::string message = Logger::formatString(format, args ...);

    Logger::log(pFile, lineNr, INFO_COLOR, "[INFO]", message);
}

template<typename ... Args>
void Logger::logWarningf(const char* pFile, int lineNr, const std::string& format, Args&& ... args)
{
    const std::string message = Logger::formatString(format, args ...);

    Logger::log(pFile, lineNr, WARN_COLOR, "[WARNING]", message);
}

template<typename ... Args>
void Logger::logErrorf(const char* pFile, int lineNr, const std::string& format, Args&& ... args)
{
    const std::string message = Logger::formatString(format, args ...);

    Logger::log(pFile, lineNr, ERROR_COLOR, "[ERROR]", message);
}

template<typename ... Args>
std::string Logger::formatString(const std::string& format, Args&& ... args)
{
    // Create a char buffer to hold the formatted string and a null termination
    size_t size = (size_t)snprintf(nullptr, 0, format.c_str(), args ...) + 1;
    std::unique_ptr<char[]> buffer(DBG_NEW char[size]);

    // Format string
    snprintf(buffer.get(), size, format.c_str(), args ...);

    // Convert to std::string without null termination
    return std::string(buffer.get(), buffer.get() + size - 1);
}
