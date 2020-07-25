#pragma once

#include <Engine/Rendering/APIAbstractions/DescriptorPool.hpp>
#include <Engine/Rendering/APIAbstractions/Vulkan/DescriptorSetVK.hpp>

#include <vulkan/vulkan.h>

class DeviceVK;

class DescriptorPoolVK : public DescriptorPool
{
public:
    static DescriptorPoolVK* create(const DescriptorPoolInfo& poolInfo, DeviceVK* pDevice);

public:
    DescriptorPoolVK(VkDescriptorPool descriptorPool, const DescriptorPoolInfo& poolInfo, DeviceVK* pDevice);
    ~DescriptorPoolVK();

    inline VkDescriptorPool getDescriptorPool() { return m_DescriptorPool; }

private:
    DescriptorSetVK* dAllocateDescriptorSet(const IDescriptorSetLayout* pDescriptorSetLayout) override final;
    void dDeallocateDescriptorSet(const DescriptorSet* pDescriptorSet) override final;

private:
    VkDescriptorPool m_DescriptorPool;
    DeviceVK* m_pDevice;
};
