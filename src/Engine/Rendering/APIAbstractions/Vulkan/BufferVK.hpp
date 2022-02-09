#pragma once

#include <Engine/Rendering/APIAbstractions/IBuffer.hpp>

#include <vma/vk_mem_alloc.h>
#include <vulkan/vulkan.h>

class DeviceVK;

class BufferVK : public IBuffer
{
public:
    static BufferVK* create(const BufferInfo& bufferInfo, DeviceVK* pDevice, StagingResources* pStagingResources);

public:
    BufferVK(VkBuffer buffer, VmaAllocation allocation, DeviceVK* pDevice);
    ~BufferVK();

    inline VkBuffer getBuffer() { return m_Buffer; }
    VmaAllocationInfo getAllocationInfo() const;

private:
    static VkBufferCreateInfo convertBufferInfo(const BufferInfo& bufferInfo);
    static VmaAllocationCreateInfo writeAllocationInfo(const BufferInfo& bufferInfo);

private:
    bool mapCopyUnmap(const void* pData, VkDeviceSize size);

private:
    VkBuffer m_Buffer;
    VmaAllocation m_Allocation;

    DeviceVK* m_pDevice;
};
