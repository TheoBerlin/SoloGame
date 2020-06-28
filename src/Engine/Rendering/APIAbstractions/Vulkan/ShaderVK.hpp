#pragma once

#include <Engine/Rendering/APIAbstractions/Shader.hpp>

#include <vulkan/vulkan.h>

#include <string>

class DeviceVK;

class ShaderVK : public Shader
{
public:
    static ShaderVK* compileShader(const std::string& filePath, SHADER_TYPE shaderType, DeviceVK* pDevice);

    static VkShaderStageFlags convertShaderFlags(SHADER_TYPE shaderFlags);
    static VkShaderStageFlagBits convertShaderFlagBits(SHADER_TYPE shaderFlags);

public:
    ShaderVK(VkShaderModule shaderModule, SHADER_TYPE shaderType, DeviceVK* pDevice);
    ~ShaderVK();

    inline VkShaderModule getShaderModule() const { return m_ShaderModule; }

private:
    VkShaderModule m_ShaderModule;
    DeviceVK* m_pDevice;
};
