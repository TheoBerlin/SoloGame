#include "DescriptorCounts.hpp"

#include <algorithm>

DescriptorCounts::DescriptorCounts()
    :m_UniformBuffers(0),
    m_CombinedTextureSamplers(0)
{}

DescriptorCounts& DescriptorCounts::operator+=(const DescriptorCounts& other)
{
    m_UniformBuffers            += other.m_UniformBuffers;
    m_CombinedTextureSamplers   += other.m_CombinedTextureSamplers;
    return *this;
}

DescriptorCounts& DescriptorCounts::operator-=(const DescriptorCounts& other)
{
    // Avoid underflow
    m_UniformBuffers            = (uint32_t)std::max(0, (int)m_UniformBuffers - (int)other.m_UniformBuffers);
    m_CombinedTextureSamplers   = (uint32_t)std::max(0, (int)m_CombinedTextureSamplers - (int)other.m_CombinedTextureSamplers);
    return *this;
}

DescriptorCounts& DescriptorCounts::operator*=(uint32_t factor)
{
    m_UniformBuffers            *= factor;
    m_CombinedTextureSamplers   *= factor;
    return *this;
}

void DescriptorCounts::ceil(const DescriptorCounts& other)
{
    m_UniformBuffers            = std::max(m_UniformBuffers, other.m_UniformBuffers);
    m_CombinedTextureSamplers   = std::max(m_CombinedTextureSamplers, other.m_CombinedTextureSamplers);
}

void DescriptorCounts::setAll(uint32_t descriptorCount)
{
    m_UniformBuffers            = descriptorCount;
    m_CombinedTextureSamplers   = descriptorCount;
}

bool DescriptorCounts::contains(const DescriptorCounts& other) const
{
    return
        m_UniformBuffers            >= other.m_UniformBuffers &&
        m_CombinedTextureSamplers   >= other.m_CombinedTextureSamplers;
}

std::string DescriptorCounts::toString() const
{
    return
        "Uniform Buffers: "             + std::to_string(m_UniformBuffers) + ", " +
        "Combined texture samplers: "   + std::to_string(m_CombinedTextureSamplers);
}

uint32_t DescriptorCounts::getDescriptorTypeCount() const
{
    return
        m_UniformBuffers            != 0u +
        m_CombinedTextureSamplers   != 0u;
}
