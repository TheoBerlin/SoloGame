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

    DescriptorSetVK* allocateDescriptorSet(const IDescriptorSetLayout* pDescriptorSetLayout) override final;
    void deallocateDescriptorSet(const DescriptorSet* pDescriptorSet) override final;

    inline VkDescriptorPool getDescriptorPool() { return m_DescriptorPool; }

private:
    VkDescriptorPool m_DescriptorPool;
    DeviceVK* m_pDevice;
};
