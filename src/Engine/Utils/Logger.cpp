#include "Logger.hpp"

#include <fstream>
#include <iomanip>
#include <iostream>
#include <sstream>
#include <fcntl.h>
#include <io.h>
#include <stdio.h>
#include <stdlib.h>

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

void Logger::logInfo(const char* pFile, int lineNr, const std::string& message)
{
    Logger::log(pFile, lineNr, INFO_COLOR, "[INFO]", message);
}

void Logger::logWarning(const char* pFile, int lineNr, const std::string& message)
{
    Logger::log(pFile, lineNr, WARN_COLOR, "[INFO]", message);
}

void Logger::logError(const char* pFile, int lineNr, const std::string& message)
{
    Logger::log(pFile, lineNr, ERROR_COLOR, "[INFO]", message);
}

void Logger::log(const char* pFile, int lineNr, unsigned short color, const std::string& severity, const std::string& message)
{
    // Remove the path from the file string
    std::string fileName(pFile);
    fileName = std::string(&pFile[fileName.find_last_of('\\') + 1]);

    std::string finalText = Logger::timeToString() + " " + severity + " " + fileName + "(" + std::to_string(lineNr) + ")" + ": " + message + "\n";

    // Print to console
    SetConsoleTextAttribute(consoleHandle, color);

    printf("%s %s %s (%d): %s\n", Logger::timeToString().c_str(), severity.c_str(), fileName.c_str(), lineNr, message.c_str());

    // Set console text color to white
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
    outputStringStream << std::put_time(&tm, "%d-%m-%Y %H:%M:%S");

    return outputStringStream.str();
}
