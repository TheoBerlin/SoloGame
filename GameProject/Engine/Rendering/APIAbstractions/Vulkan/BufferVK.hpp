#pragma once

#include <Engine/Rendering/APIAbstractions/IBuffer.hpp>

#define NOMINMAX
#include <vma/vk_mem_alloc.h>
#include <vulkan/vulkan.h>

class DeviceVK;

class BufferVK : public IBuffer
{
public:
    /*  pStagingResources is optional: If the buffer has initial data and is not mappable, the staging resources will be used. If in this case pStagingResources is nullptr,
        temporary staging resources will be created and used.*/
    static BufferVK* create(const BufferInfo& bufferInfo, DeviceVK* pDevice, StagingResources* pStagingResources);

public:
    BufferVK(VkBuffer buffer, VmaAllocation allocation, DeviceVK* pDevice);
    ~BufferVK();

    inline VkBuffer getBuffer() { return m_Buffer; }

private:
    static VkBufferCreateInfo convertBufferInfo(const BufferInfo& bufferInfo);
    static VmaAllocationCreateInfo writeAllocationInfo(const BufferInfo& bufferInfo);
    static StagingResources createTemporaryStagingResources(DeviceVK* pDevice, size_t bufferSize);

private:
    bool mapCopyUnmap(const void* pData, VkDeviceSize size);

private:
    VkBuffer m_Buffer;
    VmaAllocation m_Allocation;

    DeviceVK* m_pDevice;
};
