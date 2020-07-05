#pragma once

class IDeviceCreator
{
public:
    IDeviceCreator() = default;
    virtual ~IDeviceCreator() = 0 {};

    virtual Device* createDevice(const SwapchainInfo& swapChainInfo, const Window* pWindow) = 0;
    virtual Swapchain* createSwapchain(Device* pDevice) = 0;
};
