#pragma once

class IFence
{
public:
    virtual ~IFence() = 0 {};

    virtual bool isSignaled() = 0;

    virtual bool reset() = 0;
};
