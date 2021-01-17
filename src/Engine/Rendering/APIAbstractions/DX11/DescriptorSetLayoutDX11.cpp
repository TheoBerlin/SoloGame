#include "DescriptorSetLayoutDX11.hpp"

void DescriptorSetLayoutDX11::addBindingUniformBuffer(SHADER_BINDING binding, SHADER_TYPE shaderStages)
{
    uint32_t bindingU = (uint32_t)binding;
    m_BindingShaderMap[bindingU] = shaderStages;
    m_UniformBufferSlots.push_back({bindingU, shaderStages});
}

void DescriptorSetLayoutDX11::addBindingCombinedTextureSampler(SHADER_BINDING binding, SHADER_TYPE shaderStages)
{
    uint32_t bindingU = (uint32_t)binding;
    m_BindingShaderMap[bindingU] = shaderStages;
    m_CombinedTextureSamplerSlots.push_back({bindingU, shaderStages});
}

bool DescriptorSetLayoutDX11::finalize(Device* pDevice)
{
    UNREFERENCED_VARIABLE(pDevice);

    m_UniformBufferSlots.shrink_to_fit();
    m_CombinedTextureSamplerSlots.shrink_to_fit();
    return true;
}

DescriptorCounts DescriptorSetLayoutDX11::getDescriptorCounts() const
{
    DescriptorCounts descriptorCounts = {};
    descriptorCounts.m_UniformBuffers           = (uint32_t)m_UniformBufferSlots.size();
    descriptorCounts.m_CombinedTextureSamplers  = (uint32_t)m_CombinedTextureSamplerSlots.size();

    return descriptorCounts;
}
