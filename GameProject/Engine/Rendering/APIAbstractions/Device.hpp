#pragma once

#define NOMINMAX
#include <DirectXMath.h>

struct SwapChainInfo {
    uint32_t FrameRateLimit;
    uint32_t Multisamples;
    bool Windowed;
};

class Window;

class Device
{
public:
    virtual bool init(const SwapChainInfo& swapChainInfo, Window* pWindow) = 0;
};
