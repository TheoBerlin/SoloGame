#pragma once

#include <Engine/Rendering/APIAbstractions/DescriptorSet.hpp>

#include <vulkan/vulkan.h>

class DescriptorPoolVK;
class DescriptorSetLayoutVK;
class DeviceVK;

class DescriptorSetVK : public DescriptorSet
{
public:
    DescriptorSetVK(VkDescriptorSet descriptorSet, DescriptorPoolVK* pDescriptorPool, const DescriptorSetLayoutVK* pLayout, DeviceVK* pDevice);
    ~DescriptorSetVK() = default;

    void updateUniformBufferDescriptor(SHADER_BINDING binding, IBuffer* pBuffer) override final;
    void updateSampledTextureDescriptor(SHADER_BINDING binding, Texture* pTexture) override final;
    void updateSamplerDescriptor(SHADER_BINDING binding, ISampler* pSampler) override final;

    inline VkDescriptorSet getDescriptorSet() const { return m_DescriptorSet; }

private:
    void updateDescriptor(SHADER_BINDING binding, const VkDescriptorImageInfo* pImageInfo, const VkDescriptorBufferInfo* pBufferInfo);

private:
    VkDescriptorSet m_DescriptorSet;

    DeviceVK* m_pDevice;
};