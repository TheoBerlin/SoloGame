#pragma once

#include <Engine/Rendering/APIAbstractions/Shader.hpp>

#include <vulkan/vulkan.h>

#include <string>

class DeviceVK;

class ShaderVK : public Shader
{
public:
    static ShaderVK* compileShader(const std::string& filePath, SHADER_TYPE shaderType, DeviceVK* pDevice);

public:
    ShaderVK(VkShaderModule shaderModule, SHADER_TYPE shaderType, DeviceVK* pDevice);
    ~ShaderVK();

    inline VkShaderModule getShaderModule() { return m_ShaderModule; }

private:
    VkShaderModule m_ShaderModule;
    DeviceVK* m_pDevice;
};
