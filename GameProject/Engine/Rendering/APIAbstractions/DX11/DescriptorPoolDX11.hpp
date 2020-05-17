#pragma once

#include <Engine/Rendering/APIAbstractions/DescriptorPool.hpp>
#include <Engine/Rendering/APIAbstractions/DX11/DescriptorSetDX11.hpp>

#include <unordered_map>

class DescriptorPoolDX11 : public DescriptorPool
{
public:
    DescriptorPoolDX11() = default;
    ~DescriptorPoolDX11() = default;

    void init(const DescriptorCounts& descriptorCounts) override final;
    DescriptorSetDX11* allocateDescriptorSet(const IDescriptorSetLayout* pDescriptorSetLayout) override final;
    void deallocateDescriptorSet(const DescriptorSet* pDescriptorSet) override final;
};
