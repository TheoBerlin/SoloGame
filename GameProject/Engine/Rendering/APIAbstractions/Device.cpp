#include "Device.hpp"

#include <Engine/Rendering/APIAbstractions/DX11/DeviceDX11.hpp>
#include <Engine/Rendering/APIAbstractions/Texture.hpp>
#include <Engine/Rendering/APIAbstractions/Vulkan/DeviceVK.hpp>
#include <Engine/Utils/Debug.hpp>
#include <Engine/Utils/Logger.hpp>

Device* Device::create(RENDERING_API API)
{
    switch (API) {
        case RENDERING_API::DIRECTX11:
            return new DeviceDX11();
        case RENDERING_API::VULKAN:
            return new DeviceVK();
        default:
            LOG_ERROR("Erroneous API: %d", (int)API);
            return nullptr;
    }
}

Device::Device()
    :m_pBackBuffer(nullptr),
    m_pDepthTexture(nullptr),
    m_pShaderHandler(nullptr)
{}

Device::~Device()
{
    delete m_pBackBuffer;
    delete m_pDepthTexture;
    delete m_pShaderHandler;
}

bool Device::finalize(const DescriptorCounts& descriptorCounts)
{
    m_DescriptorPoolHandler.init(descriptorCounts, this);

    m_pShaderHandler = DBG_NEW ShaderHandler(this);
    return m_pShaderHandler;
}

DescriptorSet* Device::allocateDescriptorSet(const IDescriptorSetLayout* pDescriptorSetLayout)
{
    return m_DescriptorPoolHandler.allocateDescriptorSet(pDescriptorSetLayout, this);
}

Shader* Device::createShader(SHADER_TYPE shaderType, const std::string& filePath, const InputLayoutInfo* pInputLayoutInfo, InputLayout** ppInputLayout)
{
    // Create full path to shader file
    // Base shaders folder + relative path and name of shader + postfix and file extension
    std::string shaderPath = std::string(SHADERS_FOLDER_PATH) + filePath + getShaderPostfixAndExtension(shaderType);

    Shader* pShader = nullptr;

    do {
        pShader = compileShader(shaderType, shaderPath, pInputLayoutInfo, ppInputLayout);

        if (!pShader) {
            LOG_ERROR("Failed to compile [%s]", shaderPath.c_str());
            LOG_INFO("Edit the shader code and press any key to reattempt a compilation");
            std::getchar();
        }
    } while (!pShader);

    if (ppInputLayout && !*ppInputLayout) {
        LOG_ERROR("Failed to create input layout, shader: %s", filePath.c_str());
    }

    return pShader;
}
