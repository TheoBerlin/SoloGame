#pragma once

#include <vulkan/vulkan.h>

class DeviceVK;

class SemaphoreVK : public ISemaphore
{
public:
    static SemaphoreVK* create(DeviceVK* pDevice);

public:
    SemaphoreVK(VkSemaphore semaphore, DeviceVK* pDevice);
    ~SemaphoreVK();

    inline VkSemaphore getSemaphore() { return m_Semaphore; }

private:
    VkSemaphore m_Semaphore;
    DeviceVK* m_pDevice;
};
