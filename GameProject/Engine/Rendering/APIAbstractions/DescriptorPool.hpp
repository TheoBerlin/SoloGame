#pragma once

#include <Engine/Rendering/APIAbstractions/DescriptorCounts.hpp>

class DescriptorSet;
class IDescriptorSetLayout;

struct DescriptorPoolInfo {
    bool FreeableDescriptorSets;
    DescriptorCounts DescriptorCounts;
    uint32_t MaxSetAllocations;
};

class DescriptorPool
{
public:
    DescriptorPool(const DescriptorPoolInfo& descriptorPoolInfo);
    virtual ~DescriptorPool() = 0 {};

    virtual DescriptorSet* allocateDescriptorSet(const IDescriptorSetLayout* pDescriptorSetLayout) = 0;
    virtual void deallocateDescriptorSet(const DescriptorSet* pDescriptorSet) = 0;

    // Increase or decrease the capacity after allocating or deallocating a descripotr set
    void allocatedDescriptorSet(const DescriptorCounts& descriptorCounts);
    void deallocatedDescriptorSet(const DescriptorCounts& descriptorCounts);

    bool hasRoomFor(const DescriptorCounts& descriptorCounts);

protected:
    DescriptorCounts m_AvailableDescriptors;

private:
    // The amount of additional descriptor sets the pool is able to allocate
    uint32_t m_DescriptorSetCapacity;
};
