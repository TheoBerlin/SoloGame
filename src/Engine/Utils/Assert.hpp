#pragma once

#ifdef PLATFORM_WINDOWS
    #include <Windows.h>
#endif

#include <Engine/Utils/Logger.hpp>

#ifdef CONFIG_DEBUG
    #define ASSERT_MSG(condition, pFormat, ...) if (!(condition)) TriggerAssert(__FILE__, __LINE__, pFormat, __VA_ARGS__)
#else
    #define ASSERT_MSG(condition, format, ...)
#endif

template<typename ... Args>
inline void TriggerAssert(const char* pFile, int line, const char* pFormat, Args&& ... args)
{
    const std::string message = "Assert Triggered at " + std::string(pFile) + ":" + std::to_string(line) + "\n" +
        Logger::formatString(std::string(pFormat), args ...);

    LOG_ERROR(message);

#ifdef PLATFORM_WINDOWS
    ::MessageBoxA(0, message.c_str(), "Assert Triggered", MB_OK | MB_ICONERROR);
    __debugbreak();
#else
    abort();
#endif
}
