#pragma once

#include <Engine/Rendering/APIAbstractions/GeneralResources.hpp>

class ISemaphore;
class Texture;

class Swapchain
{
public:
    Swapchain(const Texture* const * ppDepthTextures);
    virtual ~Swapchain();

    virtual void present(ISemaphore** ppWaitSemaphores, uint32_t waitSemaphoreCount) = 0;

    virtual Texture* getBackbuffer(uint32_t frameIndex) = 0;
    inline Texture* getDepthTexture(uint32_t frameIndex) { return m_ppDepthTextures[frameIndex]; }

protected:
    Texture* m_ppDepthTextures[MAX_FRAMES_IN_FLIGHT];
};
