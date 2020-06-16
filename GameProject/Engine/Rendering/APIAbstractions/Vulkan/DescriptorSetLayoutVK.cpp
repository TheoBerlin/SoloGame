#include "DescriptorSetLayoutVK.hpp"

#include <Engine/Rendering/APIAbstractions/Vulkan/DeviceVK.hpp>
#include <Engine/Rendering/APIAbstractions/Vulkan/ShaderVK.hpp>

bool DescriptorSetLayoutVK::finalize(Device* pDevice)
{
    m_pDevice = reinterpret_cast<DeviceVK*>(pDevice);

    VkDescriptorSetLayoutCreateInfo layoutInfo = {};
    layoutInfo.sType        = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_BINDING_FLAGS_CREATE_INFO;
    layoutInfo.bindingCount = (uint32_t)m_Bindings.size();
    layoutInfo.pBindings    = m_Bindings.data();

    VkDevice device = reinterpret_cast<DeviceVK*>(pDevice)->getDevice();
    VkDescriptorSetLayout layout = VK_NULL_HANDLE;
    if (vkCreateDescriptorSetLayout(device, &layoutInfo, nullptr, &layout) != VK_SUCCESS) {
        LOG_ERROR("Failed to create descriptor set layout");
        return false;
    }

    return true;
}

DescriptorSetLayoutVK::~DescriptorSetLayoutVK()
{
    vkDestroyDescriptorSetLayout(m_pDevice->getDevice(), m_DescriptorSetLayout, nullptr);
}

void DescriptorSetLayoutVK::addBindingUniformBuffer(SHADER_BINDING binding, SHADER_TYPE shaderStages)
{
    addBinding((uint32_t)binding, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, shaderStages);
}

void DescriptorSetLayoutVK::addBindingSampledTexture(SHADER_BINDING binding, SHADER_TYPE shaderStages)
{
    addBinding((uint32_t)binding, VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE, shaderStages);
}

void DescriptorSetLayoutVK::addBindingSampler(SHADER_BINDING binding, SHADER_TYPE shaderStages)
{
    addBinding((uint32_t)binding, VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE, shaderStages);
}

void DescriptorSetLayoutVK::addBinding(uint32_t binding, VkDescriptorType descriptorType, SHADER_TYPE shaderStages)
{
    VkDescriptorSetLayoutBinding bindingInfo = {};
    bindingInfo.binding         = binding;
    bindingInfo.descriptorType  = descriptorType;
    bindingInfo.descriptorCount = 1u;
    bindingInfo.stageFlags      = ShaderVK::convertShaderFlags(shaderStages);

    m_Bindings.push_back(bindingInfo);
}
