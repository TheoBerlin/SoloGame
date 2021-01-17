#include "CommandPoolDX11.hpp"

#include <Engine/Rendering/APIAbstractions/DX11/CommandListDX11.hpp>

CommandPoolDX11::CommandPoolDX11(DeviceDX11* pDevice)
    :m_pDevice(pDevice)
{}

bool CommandPoolDX11::allocateCommandLists(ICommandList** ppCommandLists, uint32_t commandListCount, COMMAND_LIST_LEVEL level)
{
    UNREFERENCED_VARIABLE(level);

    for (uint32_t commandListIdx = 0u; commandListIdx < commandListCount; commandListIdx++) {
        ppCommandLists[commandListIdx] = nullptr;
        ppCommandLists[commandListIdx] = CommandListDX11::create(m_pDevice);

        if (!ppCommandLists[commandListIdx]) {
            return false;
        }
    }

    return true;
}
