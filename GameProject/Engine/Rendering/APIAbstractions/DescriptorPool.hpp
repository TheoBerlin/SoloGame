#pragma once

#include <Engine/Rendering/APIAbstractions/DescriptorCounts.hpp>

class DescriptorSet;
class IDescriptorSetLayout;

class DescriptorPool
{
public:
    virtual ~DescriptorPool() = 0 {};

    virtual void init(const DescriptorCounts& descriptorCounts) = 0;

    virtual DescriptorSet* allocateDescriptorSet(const IDescriptorSetLayout* pDescriptorSetLayout) = 0;
    virtual void deallocateDescriptorSet(const DescriptorSet* pDescriptorSet) = 0;

    bool hasRoomFor(const DescriptorCounts& descriptorCounts);

protected:
    DescriptorCounts m_AvailableDescriptors;
};
