#pragma once

#include <Engine/Rendering/APIAbstractions/DescriptorSetLayout.hpp>

#include <vulkan/vulkan.h>

class DeviceVK;

struct BindingSlot {
    uint32_t Binding;
    SHADER_TYPE ShaderStages;
};

class DescriptorSetLayoutVK : public IDescriptorSetLayout
{
public:
    DescriptorSetLayoutVK() = default;
    ~DescriptorSetLayoutVK();

    bool finalize(Device* pDevice) override final;

    void addBindingUniformBuffer(SHADER_BINDING binding, SHADER_TYPE shaderStages) override final;
    void addBindingSampledTexture(SHADER_BINDING binding, SHADER_TYPE shaderStages) override final;
    void addBindingSampler(SHADER_BINDING binding, SHADER_TYPE shaderStages) override final;

    DescriptorCounts getDescriptorCounts() const override final;

private:
    void addBinding(uint32_t binding, VkDescriptorType descriptorType, SHADER_TYPE shaderStages);

private:
    VkDescriptorSetLayout m_DescriptorSetLayout;
    std::vector<VkDescriptorSetLayoutBinding> m_Bindings;

    DeviceVK* m_pDevice;
};
