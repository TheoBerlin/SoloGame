#pragma once

#include <Engine/Rendering/APIAbstractions/Device.hpp>
#include <Engine/Rendering/APIAbstractions/Swapchain.hpp>
#include <Engine/Rendering/APIAbstractions/Vulkan/TextureVK.hpp>

#include <vulkan/vulkan.h>

class DeviceVK;

class SwapchainVK : public Swapchain
{
public:
    SwapchainVK(VkSwapchainKHR swapchain, const Texture* const * ppBackbuffers, const Texture* const * ppDepthTextures, DeviceVK* pDevice);
    ~SwapchainVK();

    void present(ISemaphore** ppWaitSemaphores, uint32_t waitSemaphoreCount) override final;

    Texture* getBackbuffer(uint32_t frameIndex) override final { return m_ppBackbuffers[frameIndex]; }

private:
    VkSwapchainKHR m_Swapchain;

    TextureVK* m_ppBackbuffers[MAX_FRAMES_IN_FLIGHT];

    DeviceVK* m_pDevice;

    std::vector<VkSemaphore> m_WaitSemaphores;
};
