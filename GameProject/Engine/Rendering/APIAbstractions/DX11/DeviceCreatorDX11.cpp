#include "DeviceCreatorDX11.hpp"

#include <Engine/Rendering/APIAbstractions/DX11/DeviceDX11.hpp>
#include <Engine/Rendering/APIAbstractions/DX11/GeneralResourcesDX11.hpp>
#include <Engine/Rendering/Window.hpp>
#include <Engine/Utils/Debug.hpp>
#include <Engine/Utils/DirectXUtils.hpp>

Device* DeviceCreatorDX11::createDevice(const SwapchainInfo& swapChainInfo, const Window* pWindow)
{
    if (!initDeviceAndSwapChain(swapChainInfo, pWindow)) {
        return nullptr;
    }

    if (!initBackBuffers(swapChainInfo, pWindow)) {
        return nullptr;
    }

    DeviceInfoDX11 deviceInfo = {};
    deviceInfo.pBackBuffer          = m_pBackBuffer;
    deviceInfo.pDepthTexture        = m_pDepthTexture;
    deviceInfo.pDevice              = m_pDevice;
    deviceInfo.pImmediateContext    = m_pContext;
    deviceInfo.pSwapChain           = m_pSwapChain;
    deviceInfo.pDepthStencilState   = m_pDepthStencilState;
    return DBG_NEW DeviceDX11(deviceInfo);
}

bool DeviceCreatorDX11::initDeviceAndSwapChain(const SwapchainInfo& swapChainInfo, const Window* pWindow)
{
    UINT deviceFlags = 0;

    // If in debug mode, turn on D3D11 debugging
    #if defined _DEBUG
        deviceFlags |= D3D11_CREATE_DEVICE_DEBUG;
    #endif

    // Determines the order of feature levels to attempt to create
    D3D_FEATURE_LEVEL featureLevels[] = {D3D_FEATURE_LEVEL_11_1, D3D_FEATURE_LEVEL_11_0};

    // Stores the feature level that was created
    D3D_FEATURE_LEVEL createdFeatureLevel;

    // Create swap chain description
    DXGI_SWAP_CHAIN_DESC swapChainDesc = {};
    swapChainDesc.BufferDesc.Width  = (UINT)pWindow->getWidth();
    swapChainDesc.BufferDesc.Height = (UINT)pWindow->getHeight();
    swapChainDesc.BufferDesc.Format             = DXGI_FORMAT_R8G8B8A8_UNORM;
    swapChainDesc.BufferDesc.ScanlineOrdering   = DXGI_MODE_SCANLINE_ORDER_UNSPECIFIED;
    swapChainDesc.BufferDesc.Scaling            = DXGI_MODE_SCALING_UNSPECIFIED;
    swapChainDesc.BufferDesc.RefreshRate.Numerator      = (UINT)swapChainInfo.FrameRateLimit;
    swapChainDesc.BufferDesc.RefreshRate.Denominator    = 1;
    swapChainDesc.SampleDesc.Count              = (UINT)swapChainInfo.Multisamples;
    swapChainDesc.SampleDesc.Quality            = 0;
    swapChainDesc.BufferUsage   = DXGI_USAGE_RENDER_TARGET_OUTPUT | DXGI_USAGE_UNORDERED_ACCESS;
    swapChainDesc.BufferCount   = 2;
    swapChainDesc.OutputWindow  = pWindow->getHWND();
    swapChainDesc.Windowed      = swapChainInfo.Windowed;
    swapChainDesc.SwapEffect    = DXGI_SWAP_EFFECT_FLIP_DISCARD;

    // Create device
    HRESULT hr = D3D11CreateDeviceAndSwapChain(
        nullptr,
        D3D_DRIVER_TYPE_HARDWARE,
        NULL,
        deviceFlags,
        featureLevels,
        sizeof(featureLevels) / sizeof(featureLevels[0]),
        D3D11_SDK_VERSION,
        &swapChainDesc,
        &m_pSwapChain,
        &m_pDevice,
        &createdFeatureLevel,
        &m_pContext
    );

    if (FAILED(hr) || !m_pDevice || !m_pSwapChain || !m_pContext) {
        LOG_ERROR("Failed to create Device and Swap Chain: %s", hresultToString(hr).c_str());
        return false;
    }

    return true;
}

bool DeviceCreatorDX11::initBackBuffers(const SwapchainInfo& swapChainInfo, const Window* pWindow)
{
    // Create render target view from swap chain's back buffer
    ID3D11Texture2D* pBackBuffer = nullptr;
    HRESULT hr = m_pSwapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), (LPVOID*)&pBackBuffer);
    if (FAILED(hr)) {
        LOG_ERROR("Failed to retrieve Swap Chain's back buffer: %s", hresultToString(hr).c_str());
        return false;
    }

    ID3D11RenderTargetView* pBackBufferRTV = nullptr;
    hr = m_pDevice->CreateRenderTargetView(pBackBuffer, nullptr, &pBackBufferRTV);
    pBackBuffer->Release();

    if (FAILED(hr)) {
        LOG_ERROR("Failed to create Render Target: %s", hresultToString(hr).c_str());
        return false;
    }

    D3D11_TEXTURE2D_DESC backbufferDesc = {};
    pBackBuffer->GetDesc(&backbufferDesc);

    TextureInfoDX11 textureInfoDX = {};
    textureInfoDX.Dimensions  = {(uint32_t)backbufferDesc.Width, (uint32_t)backbufferDesc.Height};
    textureInfoDX.Format      = convertFormatFromDX(backbufferDesc.Format);
    textureInfoDX.LayoutFlags = TextureDX11::convertBindFlags(backbufferDesc.BindFlags);
    textureInfoDX.pRTV        = pBackBufferRTV;
    m_pBackBuffer = DBG_NEW TextureDX11(textureInfoDX);

    /* Depth stencil */
    TextureInfo textureInfo     = {};
    textureInfo.Dimensions      = {pWindow->getWidth(), pWindow->getHeight()};
    textureInfo.Format          = RESOURCE_FORMAT::D32_FLOAT;
    textureInfo.InitialLayout   = TEXTURE_LAYOUT::DEPTH_ATTACHMENT;
    textureInfo.LayoutFlags     = textureInfo.InitialLayout;

    m_pDepthTexture = TextureDX11::create(textureInfo, m_pDevice);
    if (!m_pDepthTexture) {
        LOG_ERROR("Failed to create depth texture");
        return false;
    }

    D3D11_DEPTH_STENCIL_DESC dsDesc = {};
    dsDesc.DepthEnable                  = true;
    dsDesc.DepthWriteMask               = D3D11_DEPTH_WRITE_MASK_ALL;
    dsDesc.DepthFunc                    = D3D11_COMPARISON_LESS;
    dsDesc.StencilEnable                = false;
    dsDesc.FrontFace.StencilDepthFailOp = D3D11_STENCIL_OP_KEEP;
    dsDesc.FrontFace.StencilPassOp      = D3D11_STENCIL_OP_KEEP;
    dsDesc.FrontFace.StencilFunc        = D3D11_COMPARISON_LESS;
    dsDesc.BackFace                     = dsDesc.FrontFace;

    hr = m_pDevice->CreateDepthStencilState(&dsDesc, &m_pDepthStencilState);
    if (FAILED(hr)) {
        LOG_ERROR("Failed to create depth stencil state: %s", hresultToString(hr).c_str());
        return false;
    }

    TextureDX11* pDepthTexture = reinterpret_cast<TextureDX11*>(m_pDepthTexture);
    m_pContext->ClearDepthStencilView(pDepthTexture->getDSV(), D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0, 0);

    return true;
}
