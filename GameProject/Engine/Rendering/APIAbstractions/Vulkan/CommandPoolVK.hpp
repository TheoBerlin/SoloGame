#pragma once

#include <vulkan/vulkan.h>

class DeviceVK;

class CommandPoolVK : public ICommandPool
{
public:
    static CommandPoolVK* create(COMMAND_POOL_FLAG creationFlags, uint32_t queueFamilyIndex, DeviceVK* pDevice);

public:
    CommandPoolVK(VkCommandPool commandPool, DeviceVK* pDevice);
    ~CommandPoolVK();

    bool allocateCommandLists(ICommandList** ppCommandLists, uint32_t commandListCount, COMMAND_LIST_LEVEL level) override final;

private:
    VkCommandPool m_CommandPool;
    DeviceVK* m_pDevice;
};
