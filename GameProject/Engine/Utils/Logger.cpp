#include "Logger.hpp"

#include <fstream>
#include <iomanip>
#include <iostream>
#include <sstream>
#include <Windows.h>

#include <stdio.h>
#include <io.h>
#include <fcntl.h>
#include <stdlib.h>

HANDLE Logger::consoleHandle = nullptr;
std::ofstream Logger::logFile;

void Logger::init()
{
    AllocConsole(); //debug console
    freopen_s((FILE**)stdout, "CONOUT$", "w", stdout); //just works
    consoleHandle = GetStdHandle(STD_OUTPUT_HANDLE);
    const LPCWSTR consoleTitle = L"Debug Console";
    SetConsoleTitleW(consoleTitle);
    SetConsoleCtrlHandler(Logger::consoleEventHandler, TRUE);

    logFile.open(LOG_PATH, std::ofstream::out | std::ofstream::trunc);
}

Logger::~Logger()
{
    if (consoleHandle) {
        FreeConsole();
    }
    logFile.close();
}

BOOL WINAPI Logger::consoleEventHandler(DWORD eventType)
{
    switch(eventType)
    {
        case CTRL_C_EVENT:
        case CTRL_CLOSE_EVENT:
            FreeConsole();
            consoleHandle = nullptr;
            return TRUE;
    }

    return FALSE;
}

void Logger::log(unsigned short color, const std::string prefix, const std::string text)
{
    std::string finalText = prefix + " " + Logger::timeToString() + ": " + text + "\n";

    // Print to console
    SetConsoleTextAttribute(consoleHandle, color);

    std::cout << finalText;

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
    outputStringStream << std::put_time(&tm, "%d-%m-%Y %H-%M-%S");

    return outputStringStream.str();
}
