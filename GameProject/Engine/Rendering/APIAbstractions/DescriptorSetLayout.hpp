#pragma once

#include <Engine/Rendering/APIAbstractions/DescriptorPool.hpp>
#include <Engine/Rendering/APIAbstractions/Shader.hpp>
#include <Engine/Rendering/ShaderBindings.hpp>

class Device;

class IDescriptorSetLayout
{
public:
    virtual ~IDescriptorSetLayout() = 0 {};

    virtual bool finalize(Device* pDevice) = 0;

    virtual void addBindingUniformBuffer(SHADER_BINDING binding, SHADER_TYPE shaderStages) = 0;
    virtual void addBindingSampledTexture(SHADER_BINDING binding, SHADER_TYPE shaderStages) = 0;
    virtual void addBindingSampler(SHADER_BINDING binding, SHADER_TYPE shaderStages) = 0;

    // How many descriptors of each type are involved in the layout
    virtual DescriptorCounts getDescriptorCounts() const = 0;
};
