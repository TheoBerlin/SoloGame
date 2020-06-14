#include "BufferVK.hpp"

#include <Engine/Rendering/APIAbstractions/ICommandList.hpp>
#include <Engine/Rendering/APIAbstractions/Vulkan/DeviceVK.hpp>

BufferVK* BufferVK::create(const BufferInfo& bufferInfo, DeviceVK* pDevice, StagingResources* pStagingResources)
{
    VkDevice device = pDevice->getDevice();

    VkBufferCreateInfo createInfo = convertBufferInfo(bufferInfo);
    VkBuffer buffer = VK_NULL_HANDLE;
    if (vkCreateBuffer(device, &createInfo, nullptr, &buffer) != VK_SUCCESS) {
        LOG_WARNING("Failed to create buffer");
        return nullptr;
    }

    // Buffer creation info is finished, define the memory allocation info
    VmaAllocationCreateInfo memoryInfo  = writeAllocationInfo(bufferInfo);
    VmaAllocator vulkanAllocator        = pDevice->getVulkanAllocator();
    VmaAllocation allocation            = nullptr;
    VmaAllocationInfo allocationInfo    = {};

    if (vmaAllocateMemoryForBuffer(vulkanAllocator, buffer, &memoryInfo, &allocation, &allocationInfo) != VK_SUCCESS) {
        LOG_ERROR("Failed to allocate memory for buffer");
        return nullptr;
    }

    if (vmaBindBufferMemory(vulkanAllocator, allocation, buffer) != VK_SUCCESS) {
        LOG_ERROR("Failed to bind buffer memory");
        return nullptr;
    }

    BufferVK* pBuffer = DBG_NEW BufferVK(buffer, allocation, pDevice);

    if (bufferInfo.pData) {
        if (bufferInfo.CPUAccess == BUFFER_DATA_ACCESS::WRITE) {
            if (!pBuffer->mapCopyUnmap(bufferInfo.pData, (VkDeviceSize)bufferInfo.ByteSize)) {
                return nullptr;
            }
        } else {
            // Write data to a staging buffer, then copy the data to the target buffer
            if (!pStagingResources) {
                LOG_WARNING("pStagingResources cannot be nullptr; it is needed since the buffer has initial data and is not CPU-writable");
                return pBuffer;
            }

            if (!pBuffer->mapCopyUnmap(bufferInfo.pData, (VkDeviceSize)bufferInfo.ByteSize)) {
                return nullptr;
            }

            pStagingResources->pCommandList->copyBuffer(pStagingResources->pStagingBuffer, pBuffer, bufferInfo.ByteSize);
        }
    }

    return pBuffer;
}

BufferVK::BufferVK(VkBuffer buffer, VmaAllocation allocation, DeviceVK* pDevice)
    :m_Buffer(buffer),
    m_Allocation(allocation),
    m_pDevice(pDevice)
{}

BufferVK::~BufferVK()
{
    vmaFreeMemory(m_pDevice->getVulkanAllocator(), m_Allocation);
    vkDestroyBuffer(m_pDevice->getDevice(), m_Buffer, nullptr);
}

VmaAllocationInfo BufferVK::getAllocationInfo() const
{
    VmaAllocationInfo allocationInfo = {};
    vmaGetAllocationInfo(m_pDevice->getVulkanAllocator(), m_Allocation, &allocationInfo);
    return allocationInfo;
}

VkBufferCreateInfo BufferVK::convertBufferInfo(const BufferInfo& bufferInfo)
{
    VkBufferCreateInfo createInfo = {};
    createInfo.sType    = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
    createInfo.size     = (VkDeviceSize)bufferInfo.ByteSize;

    switch (bufferInfo.Usage) {
        case BUFFER_USAGE::VERTEX_BUFFER:
            createInfo.usage = VK_BUFFER_USAGE_VERTEX_BUFFER_BIT;
            break;
        case BUFFER_USAGE::INDEX_BUFFER:
            createInfo.usage = VK_BUFFER_USAGE_INDEX_BUFFER_BIT;
            break;
        case BUFFER_USAGE::UNIFORM_BUFFER:
            createInfo.usage = VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT;
            break;
        case BUFFER_USAGE::STAGING_BUFFER:
            createInfo.usage = VK_BUFFER_USAGE_TRANSFER_SRC_BIT;
            break;
        default:
            LOG_ERROR("Invalid buffer usage flag: %d", (int)bufferInfo.Usage);
    }

    if (bufferInfo.CPUAccess != BUFFER_DATA_ACCESS::WRITE) {
        // Not every single buffer that can't be written to by the CPU will need this flag
        createInfo.usage |= VK_BUFFER_USAGE_TRANSFER_DST_BIT;
    }

    createInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
    if (bufferInfo.SharingMode == SHARING_MODE::CONCURRENT) {
        createInfo.sharingMode              = VK_SHARING_MODE_CONCURRENT;
        createInfo.queueFamilyIndexCount    = (uint32_t)bufferInfo.QueueFamilyIndices.size();
        createInfo.pQueueFamilyIndices      = bufferInfo.QueueFamilyIndices.data();
    }

    return createInfo;
}

VmaAllocationCreateInfo BufferVK::writeAllocationInfo(const BufferInfo& bufferInfo)
{
    VmaAllocationCreateInfo memoryInfo = {};

    // Usage
    if (bufferInfo.CPUAccess == BUFFER_DATA_ACCESS::NONE) {
        memoryInfo.usage = VMA_MEMORY_USAGE_GPU_ONLY;
    } else if (bufferInfo.GPUAccess == BUFFER_DATA_ACCESS::NONE) {
        memoryInfo.usage = VMA_MEMORY_USAGE_CPU_ONLY;
    } else {
        memoryInfo.usage = VMA_MEMORY_USAGE_CPU_TO_GPU;
    }

    // Required flags
    if (bufferInfo.CPUAccess == BUFFER_DATA_ACCESS::NONE) {
        memoryInfo.requiredFlags = VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT;
    } else {
        memoryInfo.requiredFlags = VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT;
    }

    if (bufferInfo.GPUAccess != BUFFER_DATA_ACCESS::NONE) {
        // AMD GPUs have memory that is device local, and is still mappable by the CPU
        memoryInfo.preferredFlags = VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT;
    }

    return memoryInfo;
}

bool BufferVK::mapCopyUnmap(const void* pData, VkDeviceSize size)
{
    VmaAllocationInfo allocationInfo = getAllocationInfo();
    VkDevice device = m_pDevice->getDevice();
    void* pMappedData = nullptr;

    if (vkMapMemory(device, allocationInfo.deviceMemory, 0u, size, 0, &pMappedData) != VK_SUCCESS) {
        LOG_WARNING("Failed to map buffer memory");
        return false;
    }

    memcpy(pMappedData, pData, size);
    vkUnmapMemory(device, allocationInfo.deviceMemory);
    return true;
}
