#include "DescriptorSet.hpp"

#include <Engine/Rendering/APIAbstractions/DescriptorPool.hpp>
#include <Engine/Rendering/APIAbstractions/DX11/DescriptorSetLayoutDX11.hpp>

DescriptorSet::DescriptorSet(DescriptorPool* pDescriptorPool, const IDescriptorSetLayout* pLayout)
    :m_pDescriptorPool(pDescriptorPool),
    m_pLayout(pLayout)
{}

DescriptorSet::~DescriptorSet()
{
    m_pDescriptorPool->deallocateDescriptorSet(this, m_pLayout->getDescriptorCounts());
}
