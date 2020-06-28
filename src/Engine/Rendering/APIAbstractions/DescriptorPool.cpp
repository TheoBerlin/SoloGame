#include "DescriptorPool.hpp"

DescriptorPool::DescriptorPool(const DescriptorPoolInfo& descriptorPoolInfo)
    :m_AvailableDescriptors(descriptorPoolInfo.DescriptorCounts),
    m_DescriptorSetCapacity(descriptorPoolInfo.MaxSetAllocations)
{}

void DescriptorPool::allocatedDescriptorSet(const DescriptorCounts& descriptorCounts)
{
    m_AvailableDescriptors -= descriptorCounts;
    m_DescriptorSetCapacity += 1u;
}

void DescriptorPool::deallocatedDescriptorSet(const DescriptorCounts& descriptorCounts)
{
    m_AvailableDescriptors += descriptorCounts;
    m_DescriptorSetCapacity -= 1u;
}

bool DescriptorPool::hasRoomFor(const DescriptorCounts& descriptorCounts)
{
    return m_AvailableDescriptors.contains(descriptorCounts);
}
