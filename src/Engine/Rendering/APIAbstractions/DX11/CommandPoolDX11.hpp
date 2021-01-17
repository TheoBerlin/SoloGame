#pragma once

class DeviceDX11;

class CommandPoolDX11 : public ICommandPool
{
public:
    CommandPoolDX11(DeviceDX11* pDevice);
    ~CommandPoolDX11() = default;

    bool allocateCommandLists(ICommandList** ppCommandLists, uint32_t commandListCount, COMMAND_LIST_LEVEL level) override final;

private:
    DeviceDX11* m_pDevice;
};
