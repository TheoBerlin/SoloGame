#include "ShaderVK.hpp"

#include <Engine/Rendering/APIAbstractions/Vulkan/DeviceVK.hpp>

#include <fstream>

ShaderVK* ShaderVK::compileShader(const std::string& filePath, SHADER_TYPE shaderType, DeviceVK* pDevice)
{
    // Read the SPV code from file, start at the back to get the size of it
    std::ifstream file(filePath, std::ios::ate | std::ios::binary);
    if (!file.is_open()) {
        LOG_ERROR("Failed to read shader: %s", filePath.c_str());
        return nullptr;
    }

    std::vector<char> code((size_t)file.tellg());

    file.seekg(0);
    file.read(code.data(), code.size());
    file.close();

    // Use the code to create a shader module
    VkShaderModuleCreateInfo shaderInfo = {};
    shaderInfo.sType    = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
    shaderInfo.codeSize = code.size();
    shaderInfo.pCode    = (const uint32_t*)code.data();

    VkShaderModule shaderModule = VK_NULL_HANDLE;
    if (vkCreateShaderModule(pDevice->getDevice(), &shaderInfo, nullptr, &shaderModule) != VK_SUCCESS) {
        LOG_ERROR("Failed to create shader module for shader: %s", filePath.c_str());
        return nullptr;
    }

    return DBG_NEW ShaderVK(shaderModule, shaderType, pDevice);
}

VkShaderStageFlags ShaderVK::convertShaderFlags(SHADER_TYPE shaderFlags)
{
    return
        HAS_FLAG(shaderFlags, SHADER_TYPE::VERTEX_SHADER)   * VK_SHADER_STAGE_VERTEX_BIT |
        HAS_FLAG(shaderFlags, SHADER_TYPE::HULL_SHADER)     * VK_SHADER_STAGE_TESSELLATION_CONTROL_BIT |
        HAS_FLAG(shaderFlags, SHADER_TYPE::DOMAIN_SHADER)   * VK_SHADER_STAGE_TESSELLATION_EVALUATION_BIT |
        HAS_FLAG(shaderFlags, SHADER_TYPE::GEOMETRY_SHADER) * VK_SHADER_STAGE_GEOMETRY_BIT |
        HAS_FLAG(shaderFlags, SHADER_TYPE::FRAGMENT_SHADER) * VK_SHADER_STAGE_FRAGMENT_BIT;
}

ShaderVK::ShaderVK(VkShaderModule shaderModule, SHADER_TYPE shaderType, DeviceVK* pDevice)
    :Shader(shaderType),
    m_ShaderModule(shaderModule),
    m_pDevice(pDevice)
{}

ShaderVK::~ShaderVK()
{
    vkDestroyShaderModule(m_pDevice->getDevice(), m_ShaderModule, nullptr);
}
