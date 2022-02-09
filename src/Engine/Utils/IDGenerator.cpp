#include "IDGenerator.hpp"

IDGenerator::IDGenerator()
    :m_NextFree(0)
{}

uint32_t IDGenerator::GenID()
{
    if (!m_Recycled.empty()) {
        const uint32_t newID = m_Recycled.front();
        m_Recycled.pop();

        return newID;
    }

    return m_NextFree++;
}

void IDGenerator::PopID(uint32_t ID)
{
    m_Recycled.push(ID);
}
