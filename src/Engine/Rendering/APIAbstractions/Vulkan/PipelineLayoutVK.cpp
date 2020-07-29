#include "PipelineLayoutVK.hpp"

#include <Engine/Rendering/APIAbstractions/Vulkan/DescriptorSetLayoutVK.hpp>
#include <Engine/Rendering/APIAbstractions/Vulkan/DeviceVK.hpp>

PipelineLayoutVK* PipelineLayoutVK::create(const std::vector<IDescriptorSetLayout*>& descriptorSetLayouts, DeviceVK* pDevice)
{
    std::vector<VkDescriptorSetLayout> descriptorSetLayoutsVK;
    descriptorSetLayoutsVK.reserve(descriptorSetLayouts.size());

    for (const IDescriptorSetLayout* pDescriptorSetLayout : descriptorSetLayouts) {
        descriptorSetLayoutsVK.push_back(reinterpret_cast<const DescriptorSetLayoutVK*>(pDescriptorSetLayout)->getDescriptorSetLayout());
    }

    VkPipelineLayoutCreateInfo layoutInfo = {};
    layoutInfo.sType            = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
    layoutInfo.setLayoutCount   = (uint32_t)descriptorSetLayoutsVK.size();
    layoutInfo.pSetLayouts      = descriptorSetLayoutsVK.data();

    VkPipelineLayout pipelineLayout = VK_NULL_HANDLE;
    if (vkCreatePipelineLayout(pDevice->getDevice(), &layoutInfo, nullptr, &pipelineLayout) != VK_SUCCESS) {
        LOG_ERROR("Failed to create pipeline layout");
        return nullptr;
    }

    return DBG_NEW PipelineLayoutVK(pipelineLayout, pDevice);
}

PipelineLayoutVK::PipelineLayoutVK(VkPipelineLayout pipelineLayout, DeviceVK* pDevice)
    :m_PipelineLayout(pipelineLayout),
    m_pDevice(pDevice)
{}

PipelineLayoutVK::~PipelineLayoutVK()
{
    vkDestroyPipelineLayout(m_pDevice->getDevice(), m_PipelineLayout, nullptr);
}
