#include "DeviceCreatorDX11.hpp"

#include <Engine/Rendering/APIAbstractions/DX11/DeviceDX11.hpp>
#include <Engine/Rendering/APIAbstractions/DX11/GeneralResourcesDX11.hpp>
#include <Engine/Rendering/APIAbstractions/DX11/SwapchainDX11.hpp>
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
    deviceInfo.pDevice              = m_pDevice;
    deviceInfo.pImmediateContext    = m_pContext;
    deviceInfo.pDepthStencilState   = m_pDepthStencilState;
    return DBG_NEW DeviceDX11(deviceInfo);
}

Swapchain* DeviceCreatorDX11::createSwapchain(Device* pDevice)
{
    SwapchainInfoDX11 swapchainInfo = {};
    swapchainInfo.pSwapchain        = m_pSwapChain;
    swapchainInfo.pBackbuffer       = m_pBackbuffer;
    swapchainInfo.ppDepthTextures   = m_ppDepthTextures;

    return SwapchainDX11::create(swapchainInfo, reinterpret_cast<DeviceDX11*>(pDevice));
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
    swapChainDesc.BufferCount   = MAX_FRAMES_IN_FLIGHT;
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
        LOG_ERRORF("Failed to create Device and Swap Chain: %s", hresultToString(hr).c_str());
        return false;
    }

    return true;
}

bool DeviceCreatorDX11::initBackBuffers(const SwapchainInfo& swapChainInfo, const Window* pWindow)
{
    UNREFERENCED_VARIABLE(swapChainInfo);

    // Create render target views from swap chain's back buffers.
    // All of the swapchain's buffers in DirectX 11 are hidden behind the same buffer pointer.
    // Hence only one pointer is retrieved, and the same texture pointers are used each frame.
    ID3D11Texture2D* pBackBuffer = nullptr;
    HRESULT hr = m_pSwapChain->GetBuffer(0u, __uuidof(ID3D11Texture2D), (LPVOID*)&pBackBuffer);
    if (FAILED(hr)) {
        LOG_ERRORF("Failed to retrieve Swap Chain's back buffer: %s", hresultToString(hr).c_str());
        return false;
    }

    ID3D11RenderTargetView* pBackBufferRTV = nullptr;
    hr = m_pDevice->CreateRenderTargetView(pBackBuffer, nullptr, &pBackBufferRTV);
    if (FAILED(hr)) {
        LOG_ERRORF("Failed to create Render Target: %s", hresultToString(hr).c_str());
        return false;
    }

    D3D11_TEXTURE2D_DESC backbufferDesc = {};
    pBackBuffer->GetDesc(&backbufferDesc);
    pBackBuffer->Release();

    TextureInfoDX11 backbufferTextureInfo = {};
    backbufferTextureInfo.Dimensions    = { (uint32_t)backbufferDesc.Width, (uint32_t)backbufferDesc.Height };
    backbufferTextureInfo.Format        = convertFormatFromDX(backbufferDesc.Format);
    backbufferTextureInfo.pRTV          = pBackBufferRTV;

    m_pBackbuffer = DBG_NEW TextureDX11(backbufferTextureInfo);

    /* Depth stencil */
    TextureInfo depthTextureInfo    = {};
    depthTextureInfo.Dimensions     = {pWindow->getWidth(), pWindow->getHeight()};
    depthTextureInfo.Usage          = TEXTURE_USAGE::DEPTH_STENCIL;
    depthTextureInfo.Layout         = TEXTURE_LAYOUT::DEPTH_ATTACHMENT;
    depthTextureInfo.Format         = RESOURCE_FORMAT::D32_FLOAT;

    for (uint32_t depthTextureIdx = 0u; depthTextureIdx < MAX_FRAMES_IN_FLIGHT; depthTextureIdx += 1u) {
        TextureDX11* pDepthTexture = TextureDX11::create(depthTextureInfo, m_pDevice);
        if (!pDepthTexture) {
            LOG_ERROR("Failed to create depth texture");
            return false;
        }

        m_pContext->ClearDepthStencilView(pDepthTexture->getDSV(), D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0, 0);

        m_ppDepthTextures[depthTextureIdx] = pDepthTexture;
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
        LOG_ERRORF("Failed to create depth stencil state: %s", hresultToString(hr).c_str());
        return false;
    }

    return true;
}
