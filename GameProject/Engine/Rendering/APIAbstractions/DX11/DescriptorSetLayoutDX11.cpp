#include "DescriptorSetLayoutDX11.hpp"

void DescriptorSetLayoutDX11::addBindingUniformBuffer(SHADER_BINDING binding, SHADER_TYPE shaderStages)
{
    uint32_t bindingU = (uint32_t)binding;
    m_BindingShaderMap[bindingU] = shaderStages;
    m_UniformBufferSlots.push_back({bindingU, shaderStages});
}

void DescriptorSetLayoutDX11::addBindingSampledTexture(SHADER_BINDING binding, SHADER_TYPE shaderStages)
{
    uint32_t bindingU = (uint32_t)binding;
    m_BindingShaderMap[bindingU] = shaderStages;
    m_SampledTextureSlots.push_back({bindingU, shaderStages});
}

void DescriptorSetLayoutDX11::addBindingSampler(SHADER_BINDING binding, SHADER_TYPE shaderStages)
{
    uint32_t bindingU = (uint32_t)binding;
    m_BindingShaderMap[bindingU] = shaderStages;
    m_SamplerSlots.push_back({bindingU, shaderStages});
}

bool DescriptorSetLayoutDX11::finalize()
{
    m_UniformBufferSlots.shrink_to_fit();
    m_SampledTextureSlots.shrink_to_fit();
    m_SamplerSlots.shrink_to_fit();
    return true;
}

DescriptorCounts DescriptorSetLayoutDX11::getDescriptorCounts() const
{
    DescriptorCounts descriptorCounts       = {};
    descriptorCounts.m_UniformBuffers       = (uint32_t)m_UniformBufferSlots.size();
    descriptorCounts.m_SampledTextures      = (uint32_t)m_SampledTextureSlots.size();
    descriptorCounts.m_Samplers             = (uint32_t)m_SamplerSlots.size();

    return descriptorCounts;
}
