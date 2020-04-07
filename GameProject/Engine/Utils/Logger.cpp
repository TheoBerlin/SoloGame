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
WORD Logger::defaultAttributes = 0;

void Logger::init()
{
    // TODO: Direct all streams to file so that error messages aren't just printed in the console
    AllocConsole(); //debug console
    consoleHandle = GetStdHandle(STD_OUTPUT_HANDLE);
    const LPCWSTR consoleTitle = L"Debug Console";
    SetConsoleTitleW(consoleTitle);
    SetConsoleCtrlHandler(Logger::consoleEventHandler, TRUE);

    FILE *fDummy;
    freopen_s(&fDummy, "CONIN$", "r", stdin);
    freopen_s(&fDummy, "CONOUT$", "w", stderr);
    freopen_s(&fDummy, "CONOUT$", "w", stdout);
    std::cout.clear();
    std::clog.clear();
    std::cerr.clear();
    std::cin.clear();

    LPCWSTR conout = L"CONOUT$";
    LPCWSTR conin = L"CONIN$";

    // std::wcout, std::wclog, std::wcerr, std::wcin
    HANDLE hConOut = CreateFileW(conout, GENERIC_READ | GENERIC_WRITE, FILE_SHARE_READ | FILE_SHARE_WRITE, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
    HANDLE hConIn = CreateFileW(conin, GENERIC_READ | GENERIC_WRITE, FILE_SHARE_READ | FILE_SHARE_WRITE, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
    SetStdHandle(STD_OUTPUT_HANDLE, hConOut);
    SetStdHandle(STD_ERROR_HANDLE, hConOut);
    SetStdHandle(STD_INPUT_HANDLE, hConIn);
    std::wcout.clear();
    std::wclog.clear();
    std::wcerr.clear();
    std::wcin.clear();

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

void Logger::log(const char* pFile, int lineNr, unsigned short color, const std::string& severity, const std::string& text)
{
    // Remove the path from the file string
    std::string fileName(pFile);
    fileName = std::string(&pFile[fileName.find_last_of('\\') + 1]);

    std::string finalText = Logger::timeToString() + " " + severity + " " + fileName + "(" + std::to_string(lineNr) + ")" + ": " + text + "\n";

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
    outputStringStream << std::put_time(&tm, "%d-%m-%Y %H:%M:%S");

    return outputStringStream.str();
}
