#include "DescriptorPool.hpp"

bool DescriptorPool::hasRoomFor(const DescriptorCounts& descriptorCounts)
{
    return m_AvailableDescriptors.contains(descriptorCounts);
}
