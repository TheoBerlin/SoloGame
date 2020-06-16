#pragma once

#include <Engine/Rendering/APIAbstractions/DescriptorSetLayout.hpp>

#include <unordered_map>
#include <vector>

struct BindingSlot {
    uint32_t Binding;
    SHADER_TYPE ShaderStages;
};

class DescriptorSetLayoutDX11 : public IDescriptorSetLayout
{
public:
    DescriptorSetLayoutDX11() = default;
    ~DescriptorSetLayoutDX11() = default;

    void addBindingUniformBuffer(SHADER_BINDING binding, SHADER_TYPE shaderStages) override final;
    void addBindingSampledTexture(SHADER_BINDING binding, SHADER_TYPE shaderStages) override final;
    void addBindingSampler(SHADER_BINDING binding, SHADER_TYPE shaderStages) override final;

    bool finalize(Device* pDevice) override final;

    DescriptorCounts getDescriptorCounts() const override final;

    SHADER_TYPE getBindingShaderStages(uint32_t binding) const { return m_BindingShaderMap.find(binding)->second; }

private:
    std::vector<BindingSlot> m_UniformBufferSlots;
    std::vector<BindingSlot> m_SampledTextureSlots;
    std::vector<BindingSlot> m_SamplerSlots;

    // Maps binding slots to the shader stages they belong to
    std::unordered_map<uint32_t, SHADER_TYPE> m_BindingShaderMap;
};
