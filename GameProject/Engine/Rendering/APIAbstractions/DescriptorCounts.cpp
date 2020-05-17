#include "DescriptorCounts.hpp"

#include <algorithm>

DescriptorCounts::DescriptorCounts()
    :m_UniformBuffers(0),
    m_SampledTextures(0),
    m_Samplers(0)
{}

DescriptorCounts& DescriptorCounts::operator+=(const DescriptorCounts& other)
{
    m_UniformBuffers    += other.m_UniformBuffers;
    m_SampledTextures   += other.m_SampledTextures;
    m_Samplers          += other.m_Samplers;
    return *this;
}

DescriptorCounts& DescriptorCounts::operator-=(const DescriptorCounts& other)
{
    m_UniformBuffers    -= other.m_UniformBuffers;
    m_SampledTextures   -= other.m_SampledTextures;
    m_Samplers          -= other.m_Samplers;
    return *this;
}

DescriptorCounts& DescriptorCounts::operator*=(uint32_t factor)
{
    m_UniformBuffers    *= factor;
    m_SampledTextures   *= factor;
    m_Samplers          *= factor;
    return *this;
}

void DescriptorCounts::ceil(const DescriptorCounts& other)
{
    m_UniformBuffers    = std::max(m_UniformBuffers, other.m_UniformBuffers);
    m_SampledTextures   = std::max(m_SampledTextures, other.m_SampledTextures);
    m_Samplers          = std::max(m_Samplers, other.m_Samplers);
}

void DescriptorCounts::setAll(uint32_t descriptorCount)
{
    m_UniformBuffers    = descriptorCount;
    m_SampledTextures   = descriptorCount;
    m_Samplers          = descriptorCount;
}

bool DescriptorCounts::contains(const DescriptorCounts& other) const
{
    return
        m_UniformBuffers    >= other.m_UniformBuffers &&
        m_SampledTextures   >= other.m_SampledTextures &&
        m_Samplers          >= other.m_Samplers;
}

std::string DescriptorCounts::toString() const
{
    return
        "Uniform Buffers: "     + std::to_string(m_UniformBuffers) + ", " +
        "Sampled Textures: "    + std::to_string(m_SampledTextures) + ", " +
        "Samplers: "            + std::to_string(m_Samplers);
}
