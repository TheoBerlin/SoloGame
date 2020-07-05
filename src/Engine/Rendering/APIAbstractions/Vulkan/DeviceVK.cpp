#include "DeviceVK.hpp"

#include <Engine/Rendering/APIAbstractions/Vulkan/BufferVK.hpp>
#include <Engine/Rendering/APIAbstractions/Vulkan/CommandListVK.hpp>
#include <Engine/Rendering/APIAbstractions/Vulkan/CommandPoolVK.hpp>
#include <Engine/Rendering/APIAbstractions/Vulkan/DescriptorPoolVK.hpp>
#include <Engine/Rendering/APIAbstractions/Vulkan/DescriptorSetLayoutVK.hpp>
#include <Engine/Rendering/APIAbstractions/Vulkan/FramebufferVK.hpp>
#include <Engine/Rendering/APIAbstractions/Vulkan/GeneralResourcesVK.hpp>
#include <Engine/Rendering/APIAbstractions/Vulkan/PipelineLayoutVK.hpp>
#include <Engine/Rendering/APIAbstractions/Vulkan/PipelineVK.hpp>
#include <Engine/Rendering/APIAbstractions/Vulkan/RenderPassVK.hpp>
#include <Engine/Rendering/APIAbstractions/Vulkan/SamplerVK.hpp>
#include <Engine/Rendering/APIAbstractions/Vulkan/SemaphoreVK.hpp>
#include <Engine/Rendering/APIAbstractions/Vulkan/ShaderVK.hpp>
#include <Engine/Rendering/APIAbstractions/Vulkan/SwapchainVK.hpp>
#include <Engine/Rendering/APIAbstractions/Vulkan/TextureVK.hpp>
#include <Engine/Rendering/Window.hpp>

#define NOMINMAX
#define VMA_IMPLEMENTATION
#include <vma/vk_mem_alloc.h>
#include <vulkan/vulkan_win32.h>

DeviceVK::DeviceVK(const DeviceInfoVK& deviceInfo)
    :Device(deviceInfo.QueueFamilyIndices),
    m_Instance(deviceInfo.Instance),
    m_PhysicalDevice(deviceInfo.PhysicalDevice),
    m_Device(deviceInfo.Device),
    m_Surface(deviceInfo.Surface),
    m_Allocator(deviceInfo.Allocator),
    m_DebugMessenger(deviceInfo.DebugMessenger),
    m_QueueHandles(deviceInfo.QueueHandles)
{}

DeviceVK::~DeviceVK()
{
    #ifdef _DEBUG
        auto destroyDebugUtilsMessengerEXT = (PFN_vkDestroyDebugUtilsMessengerEXT) vkGetInstanceProcAddr(m_Instance, "vkDestroyDebugUtilsMessengerEXT");
        if (destroyDebugUtilsMessengerEXT) {
            destroyDebugUtilsMessengerEXT(m_Instance, m_DebugMessenger, nullptr);
        } else {
            LOG_ERROR("Failed to retrieve function pointer to vkDestroyDebugUtilsMessengerEXT");
        }
    #endif

    delete m_pSwapchain;

    vmaDestroyAllocator(m_Allocator);
    vkDestroyDevice(m_Device, nullptr);
    vkDestroySurfaceKHR(m_Instance, m_Surface, nullptr);
    vkDestroyInstance(m_Instance, nullptr);
}

bool DeviceVK::graphicsQueueSubmit(ICommandList* pCommandList, IFence* pFence, SemaphoreSubmitInfo& semaphoreSubmitInfo)
{
    if (!executeCommandBuffer(m_QueueHandles.Graphics, pCommandList, pFence, semaphoreSubmitInfo)) {
        LOG_WARNING("Failed to submit to graphics queue");
        return false;
    }

    return true;
}

bool DeviceVK::transferQueueSubmit(ICommandList* pCommandList, IFence* pFence, SemaphoreSubmitInfo& semaphoreSubmitInfo)
{
    if (!executeCommandBuffer(m_QueueHandles.Transfer, pCommandList, pFence, semaphoreSubmitInfo)) {
        LOG_WARNING("Failed to submit to transfer queue");
        return false;
    }

    return true;
}

bool DeviceVK::computeQueueSubmit(ICommandList* pCommandList, IFence* pFence, SemaphoreSubmitInfo& semaphoreSubmitInfo)
{
    if (!executeCommandBuffer(m_QueueHandles.Compute, pCommandList, pFence, semaphoreSubmitInfo)) {
        LOG_WARNING("Failed to submit to compute queue");
        return false;
    }

    return true;
}

void DeviceVK::map(IBuffer* pBuffer, void** ppMappedMemory)
{
    BufferVK* pBufferVK = reinterpret_cast<BufferVK*>(pBuffer);
    VmaAllocationInfo allocationInfo = pBufferVK->getAllocationInfo();

    if (vkMapMemory(m_Device, allocationInfo.deviceMemory, 0u, VK_WHOLE_SIZE, 0, ppMappedMemory) != VK_SUCCESS) {
        LOG_WARNING("Failed to map buffer memory");
    }
}

void DeviceVK::unmap(IBuffer* pBuffer)
{
    BufferVK* pBufferVK = reinterpret_cast<BufferVK*>(pBuffer);
    VmaAllocationInfo allocationInfo = pBufferVK->getAllocationInfo();

    vkUnmapMemory(m_Device, allocationInfo.deviceMemory);
}

ICommandPool* DeviceVK::createCommandPool(COMMAND_POOL_FLAG creationFlags, uint32_t queueFamilyIndex)
{
    return CommandPoolVK::create(creationFlags, queueFamilyIndex, this);
}

IDescriptorSetLayout* DeviceVK::createDescriptorSetLayout()
{
    return DBG_NEW DescriptorSetLayoutVK();
}

IFramebuffer* DeviceVK::createFramebuffer(const FramebufferInfo& framebufferInfo)
{
    return FramebufferVK::create(framebufferInfo, this);
}

IRenderPass* DeviceVK::createRenderPass(const RenderPassInfo& renderPassInfo)
{
    return RenderPassVK::create(renderPassInfo, this);
}

IPipelineLayout* DeviceVK::createPipelineLayout(std::vector<IDescriptorSetLayout*> descriptorSetLayouts)
{
    return PipelineLayoutVK::create(descriptorSetLayouts, this);
}

IPipeline* DeviceVK::createPipeline(const PipelineInfo& pipelineInfo)
{
    return PipelineVK::create(pipelineInfo, this);
}

FenceVK* DeviceVK::createFence(bool createSignaled)
{
    return FenceVK::create(createSignaled, this);
}

ISemaphore* DeviceVK::createSemaphore()
{
    return SemaphoreVK::create(this);
}

BufferVK* DeviceVK::createBuffer(const BufferInfo& bufferInfo, StagingResources* pStagingResources)
{
    if (bufferInfo.pData && !HAS_FLAG(bufferInfo.CPUAccess, BUFFER_DATA_ACCESS::WRITE) && !pStagingResources) {
        // Staging resources are needed but none are specified, create temporary ones
        ICommandList* pTempCommandList = nullptr;

        PooledResource<ICommandPool> tempCommandPool = acquireTempCommandPoolGraphics();
        if (!tempCommandPool->allocateCommandLists(&pTempCommandList, 1u, COMMAND_LIST_LEVEL::PRIMARY)) {
            LOG_WARNING("Failed to create temporary command list");
            return nullptr;
        }

        StagingResources tempStagingResources = {};
        tempStagingResources.pCommandList   = pTempCommandList;
        tempStagingResources.pStagingBuffer = createStagingBuffer(bufferInfo.pData, bufferInfo.ByteSize);

        BufferVK* pBuffer = BufferVK::create(bufferInfo, this, &tempStagingResources);

        delete tempStagingResources.pStagingBuffer;
        delete pTempCommandList;
        tempCommandPool.release();

        return pBuffer;
    }

    return BufferVK::create(bufferInfo, this, pStagingResources);
}

BufferVK* DeviceVK::createStagingBuffer(const void* pData, VkDeviceSize size)
{
    BufferInfo bufferInfo = {};
    bufferInfo.ByteSize     = size;
    bufferInfo.pData        = pData;
    bufferInfo.CPUAccess    = BUFFER_DATA_ACCESS::WRITE;
    bufferInfo.GPUAccess    = BUFFER_DATA_ACCESS::NONE;
    bufferInfo.Usage        = BUFFER_USAGE::STAGING_BUFFER;
    bufferInfo.SharingMode  = SHARING_MODE::EXCLUSIVE;

    return BufferVK::create(bufferInfo, this, nullptr);
}

Texture* DeviceVK::createTextureFromFile(const std::string& filePath)
{
    return TextureVK::createFromFile(filePath, this);
}

Texture* DeviceVK::createTexture(const TextureInfo& textureInfo)
{
    return TextureVK::create(textureInfo, this);
}

ISampler* DeviceVK::createSampler(const SamplerInfo& samplerInfo)
{
    return SamplerVK::create(samplerInfo, this);
}

bool DeviceVK::waitForFences(IFence** ppFences, uint32_t fenceCount, bool waitAll, uint64_t timeout)
{
    std::vector<VkFence> fences((size_t)fenceCount);
    for (uint32_t fenceIdx = 0u; fenceIdx < fenceCount; fenceIdx++) {
        fences[fenceIdx] = reinterpret_cast<FenceVK*>(ppFences[fenceIdx])->getFence();
    }

    return vkWaitForFences(m_Device, fenceCount, fences.data(), (VkBool32)waitAll, timeout) == VK_SUCCESS;
}

DescriptorPool* DeviceVK::createDescriptorPool(const DescriptorPoolInfo& poolInfo)
{
    return DescriptorPoolVK::create(poolInfo, this);
}

VKAPI_ATTR VkBool32 VKAPI_CALL DeviceVK::vulkanCallback(VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity, VkDebugUtilsMessageTypeFlagsEXT messageType, const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData, void* pUserData)
{
    switch (messageSeverity) {
        case VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT:
        case VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT:
            LOG_INFO("Validation layer: %s", pCallbackData->pMessage);
            return VK_FALSE;
        case VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT:
            LOG_WARNING("Validation layer: %s", pCallbackData->pMessage);
            return VK_FALSE;
        case VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT:
        default:
            LOG_ERROR("Validation layer: %s", pCallbackData->pMessage);
            return VK_FALSE;
    }
}

Shader* DeviceVK::compileShader(SHADER_TYPE shaderType, const std::string& filePath, const InputLayoutInfo* pInputLayoutInfo)
{
    return ShaderVK::compileShader(filePath, shaderType, this);
}

bool DeviceVK::executeCommandBuffer(VkQueue queue, ICommandList* pCommandList, IFence* pFence, SemaphoreSubmitInfo& semaphoreSubmitInfo)
{
    VkCommandBuffer commandBuffer = reinterpret_cast<CommandListVK*>(pCommandList)->getCommandBuffer();
    VkFence fence = pFence ? reinterpret_cast<FenceVK*>(pFence)->getFence() : VK_NULL_HANDLE;

    std::vector<VkSemaphore> waitSemaphores((size_t)semaphoreSubmitInfo.waitSemaphoreCount);
    std::vector<VkPipelineStageFlags> waitPipelineStages((size_t)semaphoreSubmitInfo.waitSemaphoreCount);
    for (uint32_t semaphoreIdx = 0u; semaphoreIdx < semaphoreSubmitInfo.waitSemaphoreCount; semaphoreIdx++) {
        waitSemaphores[semaphoreIdx]        = reinterpret_cast<SemaphoreVK*>(semaphoreSubmitInfo.ppWaitSemaphores[semaphoreIdx])->getSemaphore();
        waitPipelineStages[semaphoreIdx]    = convertPipelineStageFlags(semaphoreSubmitInfo.pWaitStageFlags[semaphoreIdx]);
    }

    std::vector<VkSemaphore> signalSemaphores((size_t)semaphoreSubmitInfo.signalSemaphoreCount);
    for (uint32_t semaphoreIdx = 0u; semaphoreIdx < semaphoreSubmitInfo.signalSemaphoreCount; semaphoreIdx++) {
        signalSemaphores[semaphoreIdx] = reinterpret_cast<SemaphoreVK*>(semaphoreSubmitInfo.ppSignalSemaphores[semaphoreIdx])->getSemaphore();
    }

    VkSubmitInfo submitInfo = {};
    submitInfo.sType                = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    submitInfo.commandBufferCount   = 1u;
    submitInfo.pCommandBuffers      = &commandBuffer;
    submitInfo.pWaitSemaphores      = waitSemaphores.data();
    submitInfo.waitSemaphoreCount   = semaphoreSubmitInfo.waitSemaphoreCount;
    submitInfo.pWaitDstStageMask    = waitPipelineStages.data();
    submitInfo.pSignalSemaphores    = signalSemaphores.data();
    submitInfo.signalSemaphoreCount = semaphoreSubmitInfo.signalSemaphoreCount;

    return vkQueueSubmit(queue, 1u, &submitInfo, fence) == VK_SUCCESS;
}
