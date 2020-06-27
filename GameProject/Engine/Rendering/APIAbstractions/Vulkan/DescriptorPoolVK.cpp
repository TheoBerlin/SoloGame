#include "DescriptorPoolVK.hpp"

#include <Engine/Rendering/APIAbstractions/Vulkan/DescriptorSetLayoutVK.hpp>
#include <Engine/Rendering/APIAbstractions/Vulkan/DescriptorSetVK.hpp>
#include <Engine/Rendering/APIAbstractions/Vulkan/DeviceVK.hpp>

DescriptorPoolVK* DescriptorPoolVK::create(const DescriptorPoolInfo& poolInfo, DeviceVK* pDevice)
{
    VkDescriptorPoolCreateInfo poolCreateInfo = {};
    poolCreateInfo.sType            = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
    poolCreateInfo.flags            = poolInfo.FreeableDescriptorSets ? VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT : 0u;
    poolCreateInfo.maxSets          = poolInfo.MaxSetAllocations;
    poolCreateInfo.poolSizeCount    = poolInfo.DescriptorCounts.getDescriptorTypeCount();

    std::vector<VkDescriptorPoolSize> descriptorCounts;
    descriptorCounts.reserve((size_t)poolCreateInfo.poolSizeCount);
    if (poolInfo.DescriptorCounts.m_UniformBuffers) {
        VkDescriptorPoolSize uniformPool = {};
        uniformPool.type            = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
        uniformPool.descriptorCount = poolInfo.DescriptorCounts.m_UniformBuffers;
        descriptorCounts.push_back(uniformPool);
    }

    if (poolInfo.DescriptorCounts.m_CombinedTextureSamplers) {
        VkDescriptorPoolSize sampledTexturesPool = {};
        sampledTexturesPool.type            = VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE;
        sampledTexturesPool.descriptorCount = poolInfo.DescriptorCounts.m_CombinedTextureSamplers;
        descriptorCounts.push_back(sampledTexturesPool);
    }

    poolCreateInfo.pPoolSizes = descriptorCounts.data();

    VkDescriptorPool descriptorPool = VK_NULL_HANDLE;
    if (vkCreateDescriptorPool(pDevice->getDevice(), &poolCreateInfo, nullptr, &descriptorPool) != VK_SUCCESS) {
        LOG_ERROR("Failed to create descriptor pool");
        return nullptr;
    }

    return DBG_NEW DescriptorPoolVK(descriptorPool, poolInfo, pDevice);
}

DescriptorPoolVK::DescriptorPoolVK(VkDescriptorPool descriptorPool, const DescriptorPoolInfo& poolInfo, DeviceVK* pDevice)
    :DescriptorPool(poolInfo),
    m_DescriptorPool(descriptorPool),
    m_pDevice(pDevice)
{}

DescriptorPoolVK::~DescriptorPoolVK()
{
    vkDestroyDescriptorPool(m_pDevice->getDevice(), m_DescriptorPool, nullptr);
}

DescriptorSetVK* DescriptorPoolVK::allocateDescriptorSet(const IDescriptorSetLayout* pDescriptorSetLayout)
{
    VkDescriptorSet descriptorSet = VK_NULL_HANDLE;
    if (vkAllocateDescriptorSets(m_pDevice->getDevice(), nullptr, &descriptorSet) != VK_SUCCESS) {
        LOG_ERROR("Failed to allocate descriptor set");
        return nullptr;
    }

    const DescriptorSetLayoutVK* pLayoutVK = reinterpret_cast<const DescriptorSetLayoutVK*>(pDescriptorSetLayout);
    return DBG_NEW DescriptorSetVK(descriptorSet, this, pLayoutVK, m_pDevice);
}

void DescriptorPoolVK::deallocateDescriptorSet(const DescriptorSet* pDescriptorSet)
{
    VkDescriptorSet descriptorSet = reinterpret_cast<const DescriptorSetVK*>(pDescriptorSet)->getDescriptorSet();
    if (vkFreeDescriptorSets(m_pDevice->getDevice(), m_DescriptorPool, 1u, &descriptorSet) != VK_SUCCESS) {
        LOG_ERROR("Failed to free descriptor set");
    }
}
