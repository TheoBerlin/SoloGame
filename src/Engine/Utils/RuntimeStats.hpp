#pragma once

#include <chrono>
#include <stdint.h>

class RuntimeStats
{
public:
    RuntimeStats();
    ~RuntimeStats() = default;

    void setFrameTime(float frameTime);

    float getAverageFrametime() const { return m_AverageFrametime; }

private:
    uint64_t m_FrameCount;
    float m_AverageFrametime;
};
