#pragma once

#include <Engine/Rendering/APIAbstractions/DescriptorPool.hpp>
#include <Engine/Rendering/APIAbstractions/DX11/DescriptorSetDX11.hpp>

class DescriptorPoolDX11 : public DescriptorPool
{
public:
    DescriptorPoolDX11(const DescriptorPoolInfo& poolInfo);
    ~DescriptorPoolDX11() = default;

private:
    DescriptorSetDX11* dAllocateDescriptorSet(const IDescriptorSetLayout* pDescriptorSetLayout) override final;
    void dDeallocateDescriptorSet(const DescriptorSet* pDescriptorSet) override final { UNREFERENCED_VARIABLE(pDescriptorSet); return; };
};
