#include "Logger.h"

#include <fstream>
#include <iomanip>
#include <iostream>
#include <sstream>
#include <Windows.h>

HANDLE Logger::consoleHandle = nullptr;
std::ofstream Logger::logFile;

void Logger::init()
{
    consoleHandle = GetStdHandle(STD_OUTPUT_HANDLE);

    logFile.open(LOG_PATH, std::ofstream::out | std::ofstream::trunc);
}

Logger::~Logger()
{
    logFile.close();
}

void Logger::log(unsigned short color, const std::string prefix, const std::string text)
{
    std::string finalText = prefix + " " + Logger::timeToString() + ": " + text + "\n";

    // Print to console
    SetConsoleTextAttribute(consoleHandle, color);

    std::cout << finalText;

    // Reset console settings
    //FlushConsoleInputBuffer(consoleHandle);
    SetConsoleTextAttribute(consoleHandle, 15);

    // Write to file
    logFile << finalText;

    logFile.flush();
}

std::string Logger::timeToString()
{
    time_t t = std::time(nullptr);
    std::tm tm;
    localtime_s(&tm, &t);

    std::ostringstream outputStringStream;
    outputStringStream << std::put_time(&tm, "%d-%m-%Y %H-%M-%S");

    return outputStringStream.str();
}
