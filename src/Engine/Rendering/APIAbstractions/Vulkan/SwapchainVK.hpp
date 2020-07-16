#pragma once

#include <Engine/Rendering/APIAbstractions/Device.hpp>
#include <Engine/Rendering/APIAbstractions/Swapchain.hpp>
#include <Engine/Rendering/APIAbstractions/Vulkan/TextureVK.hpp>
#include <Engine/Utils/EnumClass.hpp>

#include <vulkan/vulkan.h>

class DeviceVK;

struct SwapchainInfoVK {
    VkSwapchainKHR Swapchain;
    const Texture* const * ppBackbuffers;
    const Texture* const * ppDepthTextures;
    DeviceVK* pDevice;
};

class SwapchainVK : public Swapchain
{
public:
    static SwapchainVK* create(const SwapchainInfoVK& swapchainInfo);

public:
    SwapchainVK(const SwapchainInfoVK& swapchainInfo, const ISemaphore* const * ppSemaphores, IFence* pFence);
    ~SwapchainVK();

    void present(ISemaphore** ppWaitSemaphores, uint32_t waitSemaphoreCount) override final;

    Texture* getBackbuffer(uint32_t frameIndex) override final { return m_ppBackbuffers[frameIndex]; }

private:
    bool dAcquireNextBackbuffer(uint32_t& frameIndex, SYNC_OPTION syncOptions) override final;

private:
    VkSwapchainKHR m_Swapchain;

    TextureVK* m_ppBackbuffers[MAX_FRAMES_IN_FLIGHT];

    DeviceVK* m_pDevice;

    std::vector<VkSemaphore> m_WaitSemaphores;
};
