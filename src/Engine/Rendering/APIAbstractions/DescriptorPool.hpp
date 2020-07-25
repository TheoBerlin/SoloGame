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

    DescriptorSet* allocateDescriptorSet(const IDescriptorSetLayout* pDescriptorSetLayout, const DescriptorCounts& descriptorCounts);
    void deallocateDescriptorSet(const DescriptorSet* pDescriptorSet, const DescriptorCounts& descriptorCounts);

    bool hasRoomFor(const DescriptorCounts& descriptorCounts);

protected:
    DescriptorCounts m_AvailableDescriptors;

private:
    virtual DescriptorSet* dAllocateDescriptorSet(const IDescriptorSetLayout* pDescriptorSetLayout) = 0;
    virtual void dDeallocateDescriptorSet(const DescriptorSet* pDescriptorSet) = 0;

private:
    // The amount of additional descriptor sets the pool is able to allocate
    uint32_t m_DescriptorSetCapacity;
};
