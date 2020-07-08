#pragma once

#include <Engine/Rendering/APIAbstractions/GeneralResources.hpp>

class IFence;
class ISemaphore;
class Texture;

enum class SYNC_OPTION {
    FENCE       = 1,
    SEMAPHORE   = FENCE << 1
};

DEFINE_BITMASK_OPERATIONS(SYNC_OPTION)

class Swapchain
{
public:
    Swapchain(const Texture* const * ppDepthTextures, const ISemaphore* const * ppSemaphores, IFence* pFence);
    virtual ~Swapchain();

    // Specifying fence as a sync option does not mean the CPU will block inside this function. The fence needs to be retrieved
    // and waited for separately.
    virtual bool acquireNextBackbuffer(uint32_t& frameIndex, SYNC_OPTION syncOptions) = 0;
    virtual void present(ISemaphore** ppWaitSemaphores, uint32_t waitSemaphoreCount) = 0;

    virtual Texture* getBackbuffer(uint32_t frameIndex) = 0;
    inline Texture* getDepthTexture(uint32_t frameIndex)    { return m_ppDepthTextures[frameIndex]; }
    inline ISemaphore* getSemaphore(uint32_t frameIndex)    { return m_ppSemaphores[frameIndex]; }
    inline IFence* getFence()                               { return m_pFence; }

protected:
    Texture* m_ppDepthTextures[MAX_FRAMES_IN_FLIGHT];
    ISemaphore* m_ppSemaphores[MAX_FRAMES_IN_FLIGHT];
    IFence* m_pFence;
};
