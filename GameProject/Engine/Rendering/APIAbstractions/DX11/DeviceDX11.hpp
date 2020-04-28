#pragma once

#include <Engine/Rendering/APIAbstractions/Device.hpp>

class DeviceDX11 : public Device
{
public:
    DeviceDX11();
    ~DeviceDX11();

    bool init(const SwapChainInfo& swapChainInfo) override final;
};
