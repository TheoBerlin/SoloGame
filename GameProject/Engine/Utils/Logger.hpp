#pragma once

#include <memory>
#include <string>
#include <Windows.h>

#define LOG_PATH "log.txt"

#define Log_Info(format, ...) Logger::logInfo(__FILE__, __LINE__, format, __VA_ARGS__);
#define Log_Warning(format, ...) Logger::logWarning(__FILE__, __LINE__, format, __VA_ARGS__);
#define Log_Error(format, ...) Logger::logError(__FILE__, __LINE__, format, __VA_ARGS__);

class Logger
{
public:
    static void init();
    ~Logger();

    static BOOL WINAPI consoleEventHandler(DWORD eventType);

    template<typename ... Args>
    static void logInfo(const char* pFile, int line, const std::string& format, Args&& ... args);

    template<typename ... Args>
    static void logWarning(const char* pFile, int line, const std::string& format, Args&& ... args);

    template<typename ... Args>
    static void logError(const char* pFile, int line, const std::string& format, Args&& ... args);

private:
    // Prints prefix, formatted string, current time to console and file
    static void log(const char* pFile, int lineNr, unsigned short color, const std::string& severity, const std::string& text);

    template<typename ... Args>
    static std::string formatString(const std::string& format, Args&& ... args);
    static std::string timeToString();

    static HANDLE consoleHandle;

    static std::ofstream logFile;

    static WORD defaultAttributes;
};

template<typename ... Args>
void Logger::logInfo(const char* pFile, int lineNr, const std::string& format, Args&& ... args)
{
    std::string text = Logger::formatString(format, args ...);

    Logger::log(pFile, lineNr, FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE | FOREGROUND_INTENSITY, "[INFO]", text);
}

template<typename ... Args>
void Logger::logWarning(const char* pFile, int lineNr, const std::string& format, Args&& ... args)
{
    std::string text = Logger::formatString(format, args ...);

    Logger::log(pFile, lineNr, FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_INTENSITY, "[WARNING]", text);
}

template<typename ... Args>
void Logger::logError(const char* pFile, int lineNr, const std::string& format, Args&& ... args)
{
    std::string text = Logger::formatString(format, args ...);

    Logger::log(pFile, lineNr, FOREGROUND_RED | FOREGROUND_INTENSITY, "[ERROR]", text);
}

template<typename ... Args>
std::string Logger::formatString(const std::string& format, Args&& ... args)
{
    // Create a char buffer to hold the formatted string and a null termination
    size_t size = (size_t)snprintf(nullptr, 0, format.c_str(), std::forward<Args>(args) ...) + 1;
    std::unique_ptr<char[]> buffer(new char[size]);

    // Format string
    snprintf(buffer.get(), size, format.c_str(), std::forward<Args>(args) ...);

    // Convert to std::string without null termination
    return std::string(buffer.get(), buffer.get() + size - 1);
}
