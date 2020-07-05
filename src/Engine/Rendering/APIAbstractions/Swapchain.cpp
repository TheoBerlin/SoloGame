#include "Swapchain.hpp"

Swapchain::Swapchain(const Texture* const * ppDepthTextures)
{
    std::memcpy(m_ppDepthTextures, ppDepthTextures, sizeof(Texture*) * MAX_FRAMES_IN_FLIGHT);
}

Swapchain::~Swapchain()
{
    for (uint32_t depthTxIdx = 0u; depthTxIdx < MAX_FRAMES_IN_FLIGHT; depthTxIdx += 1u) {
        delete m_ppDepthTextures[depthTxIdx];
    }
}
