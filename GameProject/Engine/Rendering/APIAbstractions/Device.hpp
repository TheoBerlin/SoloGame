#pragma once

#define NOMINMAX
#include <DirectXMath.h>

struct SwapChainInfo {
    DirectX::XMFLOAT2 BackBufferResolution;
    uint32_t FrameRateLimit;
    uint32_t Multisamples;
};

class Device
{
public:
    Device();
    ~Device();

    virtual bool init(const SwapChainInfo& swapChainInfo) = 0;
};
