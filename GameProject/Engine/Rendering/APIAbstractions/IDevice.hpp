#pragma once

#define NOMINMAX
#include <DirectXMath.h>

struct SwapChainInfo {
    uint32_t FrameRateLimit;
    uint32_t Multisamples;
    bool Windowed;
};

class Window;

class IDevice
{
public:
    virtual bool init(const SwapChainInfo& swapChainInfo, Window* pWindow) = 0;

    virtual void clearBackBuffer() = 0;
    virtual void presentBackBuffer() = 0;
};
