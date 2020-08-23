#include "RuntimeStats.hpp"

#include <algorithm>

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
