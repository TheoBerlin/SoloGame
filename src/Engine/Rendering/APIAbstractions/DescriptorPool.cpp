#include "DescriptorPool.hpp"

DescriptorPool::DescriptorPool(const DescriptorPoolInfo& descriptorPoolInfo)
    :m_AvailableDescriptors(descriptorPoolInfo.DescriptorCounts),
    m_DescriptorSetCapacity(descriptorPoolInfo.MaxSetAllocations)
{}

DescriptorSet* DescriptorPool::allocateDescriptorSet(const IDescriptorSetLayout* pDescriptorSetLayout, const DescriptorCounts& descriptorCounts)
{
    DescriptorSet* pNewSet = dAllocateDescriptorSet(pDescriptorSetLayout);
    if (pNewSet) {
        m_AvailableDescriptors -= descriptorCounts;
        m_DescriptorSetCapacity -= 1u;
    }

    return pNewSet;
}

void DescriptorPool::deallocateDescriptorSet(const DescriptorSet* pDescriptorSet, const DescriptorCounts& descriptorCounts)
{
    dDeallocateDescriptorSet(pDescriptorSet);

    m_AvailableDescriptors += descriptorCounts;
    m_DescriptorSetCapacity += 1u;
}

bool DescriptorPool::hasRoomFor(const DescriptorCounts& descriptorCounts)
{
    return m_AvailableDescriptors.contains(descriptorCounts);
}
