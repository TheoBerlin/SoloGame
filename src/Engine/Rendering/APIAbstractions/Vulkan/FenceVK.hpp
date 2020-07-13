#pragma once

#include <vulkan/vulkan.h>

class DeviceVK;

class FenceVK : public IFence
{
public:
    static FenceVK* create(bool createSignaled, DeviceVK* pDevice);

public:
    FenceVK(VkFence fence, DeviceVK* pDevice);
    ~FenceVK();

    bool isSignaled() override final;

    bool reset() override final;

    inline VkFence getFence() { return m_Fence; }

private:
    VkFence m_Fence;
    DeviceVK* m_pDevice;
};
