#include "Device.hpp"

#include <Engine/Rendering/APIAbstractions/DeviceCreator.hpp>
#include <Engine/Rendering/APIAbstractions/DX11/DeviceCreatorDX11.hpp>
#include <Engine/Rendering/APIAbstractions/DX11/DeviceDX11.hpp>
#include <Engine/Rendering/APIAbstractions/Texture.hpp>
#include <Engine/Rendering/APIAbstractions/Vulkan/DeviceCreatorVK.hpp>
#include <Engine/Rendering/APIAbstractions/Vulkan/DeviceVK.hpp>
#include <Engine/Utils/Debug.hpp>
#include <Engine/Utils/Logger.hpp>

Device* Device::create(RENDERING_API API, const SwapchainInfo& swapchainInfo, const Window* pWindow)
{
    IDeviceCreator* pDeviceCreator = nullptr;

    switch (API) {
        case RENDERING_API::DIRECTX11:
            pDeviceCreator = DBG_NEW DeviceCreatorDX11();
            break;
        case RENDERING_API::VULKAN:
            pDeviceCreator = DBG_NEW DeviceCreatorVK();
            break;
        default:
            LOG_ERROR("Erroneous API: %d", (int)API);
            delete pDeviceCreator;
            return nullptr;
    }

    Device* pDevice = pDeviceCreator->createDevice(swapchainInfo, pWindow);
    delete pDeviceCreator;

    return pDevice;
}

Device::Device(QueueFamilyIndices queueFamilyIndices, Texture* pBackBuffer, Texture* pDepthTexture)
    :m_QueueFamilyIndices(queueFamilyIndices),
    m_pBackBuffer(pBackBuffer),
    m_pDepthTexture(pDepthTexture),
    m_pShaderHandler(nullptr)
{}

Device::~Device()
{
    delete m_pBackBuffer;
    delete m_pDepthTexture;
    delete m_pShaderHandler;
}

bool Device::init(const DescriptorCounts& descriptorCounts)
{
    m_DescriptorPoolHandler.init(descriptorCounts, this);

    m_pShaderHandler = DBG_NEW ShaderHandler(this);
    return m_pShaderHandler;
}

DescriptorSet* Device::allocateDescriptorSet(const IDescriptorSetLayout* pDescriptorSetLayout)
{
    return m_DescriptorPoolHandler.allocateDescriptorSet(pDescriptorSetLayout, this);
}

IBuffer* Device::createVertexBuffer(const void* pVertices, size_t vertexSize, size_t vertexCount)
{
    BufferInfo bufferInfo   = {};
    bufferInfo.ByteSize     = vertexSize * vertexCount;
    bufferInfo.pData        = pVertices;
    bufferInfo.Usage        = BUFFER_USAGE::VERTEX_BUFFER;
    bufferInfo.GPUAccess    = BUFFER_DATA_ACCESS::READ;
    bufferInfo.CPUAccess    = BUFFER_DATA_ACCESS::NONE;

    return createBuffer(bufferInfo);
}

IBuffer* Device::createIndexBuffer(const unsigned* pIndices, size_t indexCount)
{
    BufferInfo bufferInfo   = {};
    bufferInfo.ByteSize     = sizeof(unsigned) * indexCount;
    bufferInfo.pData        = pIndices;
    bufferInfo.Usage        = BUFFER_USAGE::INDEX_BUFFER;
    bufferInfo.GPUAccess    = BUFFER_DATA_ACCESS::READ;
    bufferInfo.CPUAccess    = BUFFER_DATA_ACCESS::NONE;

    return createBuffer(bufferInfo);
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
