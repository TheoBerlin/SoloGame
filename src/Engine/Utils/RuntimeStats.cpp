#include "RuntimeStats.hpp"

#include <algorithm>

#ifdef _WIN32
    #include <Psapi.h>
    #include <Windows.h>
#endif

RuntimeStats::RuntimeStats()
    :m_FrameCount(0u),
    m_AverageFrametime(0.0f)
{}

void RuntimeStats::setFrameTime(float frameTime)
{
    if (m_FrameCount) {
        m_AverageFrametime = m_AverageFrametime + (frameTime - m_AverageFrametime) / m_FrameCount;
    }

    m_FrameCount += 1;
}

size_t RuntimeStats::getPeakMemoryUsage()
{
    #ifdef _WIN32
        PROCESS_MEMORY_COUNTERS memInfo;
        BOOL result = GetProcessMemoryInfo(GetCurrentProcess(),
            &memInfo,
            sizeof(memInfo));

        if (!result) {
            LOG_WARNING("Failed to retrieve process memory info");
            return 0;
        }

        return (size_t)memInfo.PeakWorkingSetSize;
    #else
        LOG_WARNING("Retrieving memory usage not yet supported on platforms other than Windows");
        return 0;
    #endif
}
