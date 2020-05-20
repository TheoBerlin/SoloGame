#include "DescriptorSet.hpp"

#include <Engine/Rendering/APIAbstractions/DescriptorPool.hpp>

DescriptorSet::DescriptorSet(DescriptorPool* pDescriptorPool, const IDescriptorSetLayout* pLayout)
    :m_pDescriptorPool(pDescriptorPool),
    m_pLayout(pLayout)
{}

DescriptorSet::~DescriptorSet()
{
    m_pDescriptorPool->deallocateDescriptorSet(this);
}
