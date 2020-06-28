#pragma once

class FenceDX11 : public IFence
{
public:
    FenceDX11() = default;
    ~FenceDX11() = default;

    bool isSignaled() override final { return true; }
};
