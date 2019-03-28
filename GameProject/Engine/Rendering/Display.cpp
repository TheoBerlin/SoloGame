#include "Display.hpp"

#include <WinUser.h>

ID3D11Device* Display::device = nullptr;

Display::~Display()
{
    device->Release();
}

void Display::init(unsigned int height, float aspectRatio)
{
    HRESULT hr;

    // Singlethreaded flag improves performance when D3D11 calls are only made on one thread
    UINT deviceFlags = D3D11_CREATE_DEVICE_SINGLETHREADED;

    // If in debug mode, turn on D3D11 debugging
    #if defined _DEBUG
        deviceFlags |= D3D11_CREATE_DEVICE_DEBUG | D3D11_CREATE_DEVICE_DEBUGGABLE;
    #endif

    // Determines the order of feature levels to attempt to create
    D3D_FEATURE_LEVEL featureLevels[] = {D3D_FEATURE_LEVEL_11_1, D3D_FEATURE_LEVEL_11_0};

    // Create swap chain description
    DXGI_SWAP_CHAIN_DESC swapChainDesc;

    DXGI_MODE_DESC displayMode;
    // Use the size of the active window
    displayMode.Width = 0;
    displayMode.Height = 0;


    swapChainDesc.Width = (UINT)(height * aspectRatio);

    hr = D3D11CreateDeviceAndSwapChain(
        nullptr, // Use default video adapter
        D3D_DRIVER_TYPE_HARDWARE,
        NULL, // No software rasterizer is used
        deviceFlags,
        featureLevels,
        sizeof(featureLevels) / sizeof(featureLevels[0]),
        D3D11_SDK_VERSION,

    )
}

ID3D11Device* Display::getDevice()
{
    return device;
}
