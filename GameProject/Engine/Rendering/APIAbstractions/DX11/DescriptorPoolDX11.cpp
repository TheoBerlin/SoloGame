#include "DescriptorPoolDX11.hpp"

#include <Engine/Rendering/APIAbstractions/DX11/DescriptorSetLayoutDX11.hpp>

void DescriptorPoolDX11::init(const DescriptorCounts& descriptorCounts)
{
    m_AvailableDescriptors = descriptorCounts;
}

DescriptorSetDX11* DescriptorPoolDX11::allocateDescriptorSet(const IDescriptorSetLayout* pDescriptorSetLayout)
{
    m_AvailableDescriptors -= pDescriptorSetLayout->getDescriptorCounts();

    const DescriptorSetLayoutDX11* pDescriptorSetLayoutDX = reinterpret_cast<const DescriptorSetLayoutDX11*>(pDescriptorSetLayout);
    return new DescriptorSetDX11(pDescriptorSetLayoutDX, this);
}

void DescriptorPoolDX11::deallocateDescriptorSet(const DescriptorSet* pDescriptorSet)
{
    m_AvailableDescriptors += pDescriptorSet->getLayout()->getDescriptorCounts();
    return;
}
