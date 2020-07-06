#include "DescriptorSetVK.hpp"

#include <Engine/Rendering/APIAbstractions/Vulkan/DescriptorPoolVK.hpp>
#include <Engine/Rendering/APIAbstractions/Vulkan/DescriptorSetLayoutVK.hpp>
#include <Engine/Rendering/APIAbstractions/Vulkan/DeviceVK.hpp>
#include <Engine/Rendering/APIAbstractions/Vulkan/SamplerVK.hpp>
#include <Engine/Rendering/APIAbstractions/Vulkan/TextureVK.hpp>

DescriptorSetVK::DescriptorSetVK(VkDescriptorSet descriptorSet, DescriptorPoolVK* pDescriptorPool, const DescriptorSetLayoutVK* pLayout, DeviceVK* pDevice)
    :DescriptorSet(pDescriptorPool, pLayout),
    m_DescriptorSet(descriptorSet),
    m_pDevice(pDevice)
{}

void DescriptorSetVK::updateUniformBufferDescriptor(SHADER_BINDING binding, IBuffer* pBuffer)
{
    VkDescriptorBufferInfo bufferInfo = {};
    bufferInfo.buffer = reinterpret_cast<BufferVK*>(pBuffer)->getBuffer();
    bufferInfo.range  = VK_WHOLE_SIZE;

    updateDescriptor(binding, nullptr, &bufferInfo, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER);
}

void DescriptorSetVK::updateCombinedTextureSamplerDescriptor(SHADER_BINDING binding, Texture* pTexture, ISampler* pSampler)
{
    VkDescriptorImageInfo imageInfo = {};
    imageInfo.sampler       = reinterpret_cast<SamplerVK*>(pSampler)->getSampler();
    imageInfo.imageView     = reinterpret_cast<TextureVK*>(pTexture)->getImageView();
    imageInfo.imageLayout   = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;

    updateDescriptor(binding, &imageInfo, nullptr, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER);
}

void DescriptorSetVK::updateDescriptor(SHADER_BINDING binding, const VkDescriptorImageInfo* pImageInfo, const VkDescriptorBufferInfo* pBufferInfo, VkDescriptorType descriptorType)
{
    VkWriteDescriptorSet writeInfo = {};
    writeInfo.sType             = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
    writeInfo.dstSet            = m_DescriptorSet;
    writeInfo.dstBinding        = (uint32_t)binding;
    writeInfo.dstArrayElement   = 0u;
    writeInfo.descriptorCount   = 1u;
    writeInfo.descriptorType    = descriptorType;
    writeInfo.pImageInfo        = pImageInfo;
    writeInfo.pBufferInfo       = pBufferInfo;

    vkUpdateDescriptorSets(m_pDevice->getDevice(), 1u, &writeInfo, 0u, nullptr);
}
