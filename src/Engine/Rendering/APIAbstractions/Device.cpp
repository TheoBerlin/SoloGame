#include "Device.hpp"

#include <Engine/Rendering/APIAbstractions/DeviceCreator.hpp>
#include <Engine/Rendering/APIAbstractions/DX11/DeviceCreatorDX11.hpp>
#include <Engine/Rendering/APIAbstractions/DX11/DeviceDX11.hpp>
#include <Engine/Rendering/APIAbstractions/Swapchain.hpp>
#include <Engine/Rendering/APIAbstractions/Texture.hpp>
#include <Engine/Rendering/APIAbstractions/Vulkan/DeviceCreatorVK.hpp>
#include <Engine/Rendering/APIAbstractions/Vulkan/DeviceVK.hpp>
#include <Engine/Utils/Debug.hpp>
#include <Engine/Utils/Logger.hpp>

#include <thread>

Device* Device::create(RENDERING_API API, const SwapchainInfo& swapchainInfo, const Window* pWindow)
{
    std::unique_ptr<IDeviceCreator> pDeviceCreator(nullptr);

    switch (API) {
        case RENDERING_API::DIRECTX11:
            pDeviceCreator.reset(DBG_NEW DeviceCreatorDX11());
            break;
        case RENDERING_API::VULKAN:
            pDeviceCreator.reset(DBG_NEW DeviceCreatorVK());
            break;
        default:
            LOG_ERROR("Erroneous API: %d", (int)API);
            return nullptr;
    }

    std::unique_ptr<Device> pDevice(pDeviceCreator->createDevice(swapchainInfo, pWindow));

    // Temporary command pools are required for swapchain creation
    if (!pDevice->initTempCommandPools()) {
        return nullptr;
    }

    Swapchain* pSwapchain = pDeviceCreator->createSwapchain(pDevice.get());
    if (!pSwapchain) {
        return nullptr;
    }

    pDevice->setSwapchain(pSwapchain);

    return pDevice.release();
}

Device::Device(QueueFamilyIndices queueFamilyIndices)
    :m_QueueFamilyIndices(queueFamilyIndices),
    m_pSwapchain(nullptr),
    m_pShaderHandler(nullptr),
    m_FrameIndex(0u)
{}

Device::~Device()
{
    delete m_pShaderHandler;
}

bool Device::init(const DescriptorCounts& descriptorCounts)
{
    DescriptorPoolInfo descriptorPoolInfo = {};
    descriptorPoolInfo.DescriptorCounts         = descriptorCounts;
    descriptorPoolInfo.MaxSetAllocations        = 25u;
    descriptorPoolInfo.FreeableDescriptorSets   = true;
    m_DescriptorPoolHandler.init(descriptorPoolInfo, this);

    m_pShaderHandler = DBG_NEW ShaderHandler(this);
    return m_pShaderHandler;
}

void Device::presentBackbuffer(ISemaphore** ppWaitSemaphores, uint32_t waitSemaphoreCount)
{
    m_FrameIndex = (m_FrameIndex + 1u) % MAX_FRAMES_IN_FLIGHT;
    m_pSwapchain->present(ppWaitSemaphores, waitSemaphoreCount);
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

Shader* Device::createShader(SHADER_TYPE shaderType, const std::string& filePath, const InputLayoutInfo* pInputLayoutInfo)
{
    // Create full path to shader file
    // Base shaders folder + relative path and name of shader + postfix and file extension
    std::string shaderPath = std::string(SHADERS_FOLDER_PATH) + filePath + Shader::getTypePostfix(shaderType) + getShaderFileExtension();
    Shader* pShader = nullptr;

    do {
        pShader = compileShader(shaderType, shaderPath, pInputLayoutInfo);

        if (!pShader) {
            LOG_ERROR("Failed to compile [%s]", shaderPath.c_str());
            LOG_INFO("Edit the shader code and press any key to reattempt a compilation");
            std::getchar();
        }
    } while (!pShader);

    return pShader;
}

void Device::deleteGraphicsObjects()
{
    delete m_pSwapchain;
    m_DescriptorPoolHandler.clear();
    m_CommandPoolsTempGraphics.clear();
    m_CommandPoolsTempTransfer.clear();
    m_CommandPoolsTempCompute.clear();
}

bool Device::initTempCommandPools()
{
    // Create pool of command pools
    // hardware_concurrency might return 0
    unsigned int threadCount = std::thread::hardware_concurrency();
    threadCount = threadCount == 0 ? 4u : threadCount;
    std::vector<ICommandPool*> commandPools((size_t)threadCount);

    std::array<TempCommandPoolInfo, 3u> commandPoolInfos = {{
        {
            m_QueueFamilyIndices.Graphics,
            &m_CommandPoolsTempGraphics
        },
        {
            m_QueueFamilyIndices.Transfer,
            &m_CommandPoolsTempTransfer
        },
        {
            m_QueueFamilyIndices.Compute,
            &m_CommandPoolsTempCompute
        }
    }};

    for (TempCommandPoolInfo& commandPoolInfo : commandPoolInfos) {
        if (!initTempCommandPool(commandPools, commandPoolInfo)) {
            return false;
        }
    }

    return true;
}

bool Device::initTempCommandPool(std::vector<ICommandPool*>& commandPools, TempCommandPoolInfo& commandPoolInfo)
{
    for (ICommandPool*& pCommandPool : commandPools) {
        pCommandPool = createCommandPool(COMMAND_POOL_FLAG::TEMPORARY_COMMAND_LISTS, commandPoolInfo.queueFamilyIndex);

        if (!pCommandPool) {
            return false;
        }
    }

    commandPoolInfo.pTargetCommandPool->init(commandPools);
    return true;
}
