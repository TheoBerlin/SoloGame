#pragma once

#include <queue>

class IDGenerator
{
public:
    IDGenerator();
    ~IDGenerator() = default;

    uint32_t GenID();

    // Registers ID as free to be generated again
    void PopID(uint32_t ID);

private:
    uint32_t m_NextFree;

    std::queue<uint32_t> m_Recycled;
};
