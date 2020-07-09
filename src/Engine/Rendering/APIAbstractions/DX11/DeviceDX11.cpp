#include "DeviceDX11.hpp"

#include <Engine/Rendering/APIAbstractions/DX11/CommandListDX11.hpp>
#include <Engine/Rendering/APIAbstractions/DX11/CommandPoolDX11.hpp>
#include <Engine/Rendering/APIAbstractions/DX11/DescriptorSetLayoutDX11.hpp>
#include <Engine/Rendering/APIAbstractions/DX11/InputLayoutDX11.hpp>
#include <Engine/Rendering/APIAbstractions/DX11/FramebufferDX11.hpp>
#include <Engine/Rendering/APIAbstractions/DX11/GeneralResourcesDX11.hpp>
#include <Engine/Rendering/APIAbstractions/DX11/RenderPassDX11.hpp>
#include <Engine/Rendering/APIAbstractions/DX11/SwapchainDX11.hpp>
#include <Engine/Rendering/Window.hpp>
#include <Engine/Utils/Debug.hpp>
#include <Engine/Utils/DirectXUtils.hpp>
#include <Engine/Utils/Logger.hpp>

DeviceDX11::DeviceDX11(const DeviceInfoDX11& deviceInfo)
    :Device({}),
    m_pDevice(deviceInfo.pDevice),
    m_pContext(deviceInfo.pImmediateContext),
    m_pDepthStencilState(deviceInfo.pDepthStencilState)
{}

DeviceDX11::~DeviceDX11()
{
    delete m_pSwapchain;

    SAFERELEASE(m_pContext)
    SAFERELEASE(m_pDepthStencilState)
    SAFERELEASE(m_pDevice)
}

bool DeviceDX11::graphicsQueueSubmit(ICommandList* pCommandList, IFence* pFence, SemaphoreSubmitInfo& semaphoreSubmitInfo)
{
    return executeCommandList(pCommandList);
}

bool DeviceDX11::transferQueueSubmit(ICommandList* pCommandList, IFence* pFence, SemaphoreSubmitInfo& semaphoreSubmitInfo)
{
    return executeCommandList(pCommandList);
}

bool DeviceDX11::computeQueueSubmit(ICommandList* pCommandList, IFence* pFence, SemaphoreSubmitInfo& semaphoreSubmitInfo)
{
    return executeCommandList(pCommandList);
}

ICommandPool* DeviceDX11::createCommandPool(COMMAND_POOL_FLAG creationFlags, uint32_t queueFamilyIndex)
{
    return DBG_NEW CommandPoolDX11(this);
}

IDescriptorSetLayout* DeviceDX11::createDescriptorSetLayout()
{
    return DBG_NEW DescriptorSetLayoutDX11();
}

Framebuffer* DeviceDX11::createFramebuffer(const FramebufferInfo& framebufferInfo)
{
    return FramebufferDX11::create(framebufferInfo);
}

IRenderPass* DeviceDX11::createRenderPass(const RenderPassInfo& renderPassInfo)
{
    return RenderPassDX11::create(renderPassInfo);
}

PipelineDX11* DeviceDX11::createPipeline(const PipelineInfo& pipelineInfo)
{
    return PipelineDX11::create(pipelineInfo, this);
}

void DeviceDX11::map(IBuffer* pBuffer, void** ppMappedMemory)
{
    ID3D11Buffer* pBufferDX = reinterpret_cast<BufferDX11*>(pBuffer)->getBuffer();

    D3D11_MAPPED_SUBRESOURCE mappedResources = {};
    m_pContext->Map(pBufferDX, 0u, D3D11_MAP_WRITE_DISCARD, 0u, &mappedResources);
    (*ppMappedMemory) = mappedResources.pData;
}

void DeviceDX11::unmap(IBuffer* pBuffer)
{
    m_pContext->Unmap(reinterpret_cast<BufferDX11*>(pBuffer)->getBuffer(), 0u);
}

BufferDX11* DeviceDX11::createBuffer(const BufferInfo& bufferInfo, StagingResources* pStagingResources)
{
    BufferDX11* pBuffer = DBG_NEW BufferDX11(m_pDevice, bufferInfo);
    if (!pBuffer->getBuffer()) {
        delete pBuffer;
        pBuffer = nullptr;
    }

    return pBuffer;
}

TextureDX11* DeviceDX11::createTextureFromFile(const std::string& filePath)
{
    return TextureDX11::createFromFile(filePath, m_pDevice);
}

TextureDX11* DeviceDX11::createTexture(const TextureInfo& textureInfo)
{
    return TextureDX11::create(textureInfo, m_pDevice);
}

SamplerDX11* DeviceDX11::createSampler(const SamplerInfo& samplerInfo)
{
    return SamplerDX11::create(samplerInfo, m_pDevice);
}

DescriptorPoolDX11* DeviceDX11::createDescriptorPool(const DescriptorPoolInfo& poolInfo)
{
    return DBG_NEW DescriptorPoolDX11(poolInfo);
}

ShaderDX11* DeviceDX11::compileShader(SHADER_TYPE shaderType, const std::string& filePath, const InputLayoutInfo* pInputLayoutInfo)
{
    std::wstring filePathW(filePath.begin(), filePath.end());
    std::string targetVer = ShaderDX11::getTargetVersion(shaderType);

    ID3DBlob* pCompiledCode = ShaderDX11::compileShader(filePathW.c_str(), targetVer.c_str());
    if (!pCompiledCode) {
        return nullptr;
    }

    switch (shaderType) {
        case SHADER_TYPE::VERTEX_SHADER:
            return ShaderDX11::createVertexShader(shaderType, pCompiledCode, filePath, m_pDevice, pInputLayoutInfo);
        case SHADER_TYPE::HULL_SHADER:
            return ShaderDX11::createHullShader(shaderType, pCompiledCode, filePath, m_pDevice);
        case SHADER_TYPE::DOMAIN_SHADER:
            return ShaderDX11::createDomainShader(shaderType, pCompiledCode, filePath, m_pDevice);
        case SHADER_TYPE::GEOMETRY_SHADER:
            return ShaderDX11::createGeometryShader(shaderType, pCompiledCode, filePath, m_pDevice);
        case SHADER_TYPE::FRAGMENT_SHADER:
            return ShaderDX11::createFragmentShader(shaderType, pCompiledCode, filePath, m_pDevice);
        default:
            LOG_ERROR("Erroneous shader type: %d", (int)shaderType);
            return nullptr;
    }
}

bool DeviceDX11::executeCommandList(ICommandList* pCommandList)
{
    ID3D11CommandList* pCommandListDX = reinterpret_cast<CommandListDX11*>(pCommandList)->getCommandList();
    if (pCommandListDX) {
        m_pContext->ExecuteCommandList(pCommandListDX, FALSE);
    }

    return true;
}
