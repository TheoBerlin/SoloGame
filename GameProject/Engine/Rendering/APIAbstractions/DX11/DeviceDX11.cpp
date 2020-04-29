#include "DeviceDX11.hpp"

#include <Engine/Rendering/Window.hpp>
#include <Engine/Utils/DirectXUtils.hpp>
#include <Engine/Utils/Logger.hpp>

DeviceDX11::DeviceDX11()
    :m_pDevice(nullptr),
    m_pSwapChain(nullptr),
    m_pDeviceContext(nullptr),
    m_pBackBufferRTV(nullptr),
    m_pDepthStencilTX(nullptr),
    m_pDepthStencilView(nullptr),
    m_pDepthStencilState(nullptr),
    m_pBlendState(nullptr)
{}

DeviceDX11::~DeviceDX11()
{
    if (m_pDevice) {
        m_pDevice->Release();
    }

    if (m_pSwapChain) {
        m_pSwapChain->Release();
    }

    if (m_pDeviceContext) {
        m_pDeviceContext->Release();
    }

    if (m_pBackBufferRTV) {
        m_pBackBufferRTV->Release();
    }

    if (m_pBlendState) {
        m_pBlendState->Release();
    }

    if (m_pDepthStencilTX) {
        m_pDepthStencilTX->Release();
    }

    if (m_pDepthStencilView) {
        m_pDepthStencilView->Release();
    }

    if (m_pDepthStencilState) {
        m_pDepthStencilState->Release();
    }
}

bool DeviceDX11::init(const SwapChainInfo& swapChainInfo, Window* pWindow)
{
    if (!initDeviceAndSwapChain(swapChainInfo, pWindow)) {
        return false;
    }

    return initBackBuffers(swapChainInfo, pWindow);
}

bool DeviceDX11::initDeviceAndSwapChain(const SwapChainInfo& swapChainInfo, Window* pWindow)
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
    DXGI_SWAP_CHAIN_DESC swapChainDesc;
    ZeroMemory(&swapChainDesc, sizeof(DXGI_SWAP_CHAIN_DESC));
    swapChainDesc.BufferDesc.Width = (UINT)pWindow->getWidth();
    swapChainDesc.BufferDesc.Height = (UINT)pWindow->getHeight();
    swapChainDesc.BufferDesc.RefreshRate.Numerator = (UINT)swapChainInfo.FrameRateLimit;
    swapChainDesc.BufferDesc.RefreshRate.Denominator = 1;
    swapChainDesc.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
    swapChainDesc.BufferDesc.ScanlineOrdering = DXGI_MODE_SCANLINE_ORDER_UNSPECIFIED;
    swapChainDesc.BufferDesc.Scaling = DXGI_MODE_SCALING_UNSPECIFIED;
    swapChainDesc.SampleDesc.Count = (UINT)swapChainInfo.Multisamples;
    swapChainDesc.SampleDesc.Quality = 0;
    swapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT | DXGI_USAGE_UNORDERED_ACCESS;
    swapChainDesc.BufferCount = 2;
    swapChainDesc.OutputWindow = pWindow->getHWND();
    swapChainDesc.Windowed = swapChainInfo.Windowed;
    swapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;

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
        &m_pDeviceContext
    );

    if (FAILED(hr) || !m_pDevice || !m_pSwapChain || !m_pDeviceContext) {
        LOG_ERROR("Failed to create Device and Swap Chain: %s", hresultToString(hr).c_str());
        return false;
    }

    return true;
}

bool DeviceDX11::initBackBuffers(const SwapChainInfo& swapChainInfo, Window* pWindow)
{
    ID3D11Texture2D* pBackBuffer = nullptr;
    HRESULT hr = m_pSwapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), (LPVOID*)&pBackBuffer);
    if (FAILED(hr)) {
        LOG_ERROR("Failed to retrieve Swap Chain's back buffer: %s", hresultToString(hr).c_str());
        return false;
    }

    hr = m_pDevice->CreateRenderTargetView(pBackBuffer, nullptr, &m_pBackBufferRTV);
    if (FAILED(hr)) {
        LOG_ERROR("Failed to create Render Target: %s", hresultToString(hr).c_str());
        return false;
    }

    pBackBuffer->Release();

    /* Depth stencil */
    D3D11_TEXTURE2D_DESC depthTxDesc = {};
    ZeroMemory(&depthTxDesc, sizeof(D3D11_TEXTURE2D_DESC));
    depthTxDesc.Width = (UINT)pWindow->getWidth();
    depthTxDesc.Height = (UINT)pWindow->getHeight();
    depthTxDesc.MipLevels = 1;
    depthTxDesc.ArraySize = 1;
    depthTxDesc.Format = DXGI_FORMAT_D32_FLOAT;
    depthTxDesc.SampleDesc.Count = 1;
    depthTxDesc.SampleDesc.Quality = 0;
    depthTxDesc.Usage = D3D11_USAGE_DEFAULT;
    depthTxDesc.BindFlags = D3D11_BIND_DEPTH_STENCIL;
    depthTxDesc.CPUAccessFlags = 0;
    depthTxDesc.MiscFlags = 0;

    hr = m_pDevice->CreateTexture2D(&depthTxDesc, nullptr, &m_pDepthStencilTX);
    if (FAILED(hr)) {
        LOG_ERROR("Failed to create depth stencil texture: %s", hresultToString(hr).c_str());
        return false;
    }

    D3D11_DEPTH_STENCIL_DESC dsDesc = {};
    ZeroMemory(&dsDesc, sizeof(D3D11_DEPTH_STENCIL_DESC));
    dsDesc.DepthEnable = true;
    dsDesc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ALL;
    dsDesc.DepthFunc = D3D11_COMPARISON_LESS;
    dsDesc.StencilEnable = false;
    dsDesc.FrontFace.StencilDepthFailOp = D3D11_STENCIL_OP_KEEP;
    dsDesc.FrontFace.StencilPassOp = D3D11_STENCIL_OP_KEEP;
    dsDesc.FrontFace.StencilFunc = D3D11_COMPARISON_LESS;
    dsDesc.BackFace = dsDesc.FrontFace;

    hr = m_pDevice->CreateDepthStencilState(&dsDesc, &m_pDepthStencilState);
    if (FAILED(hr)) {
        LOG_ERROR("Failed to create depth stencil state: %s", hresultToString(hr).c_str());
        return false;
    }

    D3D11_DEPTH_STENCIL_VIEW_DESC dsViewDesc = {};
    ZeroMemory(&dsViewDesc, sizeof(D3D11_DEPTH_STENCIL_VIEW_DESC));
    dsViewDesc.Format = DXGI_FORMAT_D32_FLOAT;
    dsViewDesc.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D;
    dsViewDesc.Flags = 0;
    dsViewDesc.Texture2D.MipSlice = 0;

    hr = m_pDevice->CreateDepthStencilView(m_pDepthStencilTX, &dsViewDesc, &m_pDepthStencilView);
    if (FAILED(hr)) {
        LOG_ERROR("Failed to create depth stencil view: %s", hresultToString(hr).c_str());
        return false;
    }

    m_pDeviceContext->ClearDepthStencilView(m_pDepthStencilView, D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0, 0);

    /* Set viewport */
    D3D11_VIEWPORT viewPort;
    viewPort.TopLeftX = 0;
    viewPort.TopLeftY = 0;
    viewPort.Width = (float)pWindow->getWidth();
    viewPort.Height = (float)pWindow->getHeight();
    viewPort.MinDepth = 0.0f;
    viewPort.MaxDepth = 1.0f;
    m_pDeviceContext->RSSetViewports(1, &viewPort);

    // Create blend state
    D3D11_RENDER_TARGET_BLEND_DESC rtvBlendDesc = {};
    rtvBlendDesc.BlendEnable = TRUE;
    rtvBlendDesc.SrcBlend = D3D11_BLEND_ONE;
    rtvBlendDesc.DestBlend = D3D11_BLEND_INV_SRC_ALPHA;
    rtvBlendDesc.BlendOp = D3D11_BLEND_OP_ADD;
    rtvBlendDesc.SrcBlendAlpha = D3D11_BLEND_ONE;
    rtvBlendDesc.DestBlendAlpha = D3D11_BLEND_ONE;
    rtvBlendDesc.BlendOpAlpha = D3D11_BLEND_OP_ADD;
    rtvBlendDesc.RenderTargetWriteMask = D3D11_COLOR_WRITE_ENABLE_ALL;

    D3D11_BLEND_DESC blendDesc = {};
    blendDesc.AlphaToCoverageEnable = FALSE;
    blendDesc.RenderTarget[0] = rtvBlendDesc;

    hr = m_pDevice->CreateBlendState(&blendDesc, &m_pBlendState);
    if (hr != S_OK) {
        LOG_ERROR("Failed create blend state: %s", hresultToString(hr).c_str());
        return false;
    }

    m_pDeviceContext->OMSetBlendState(m_pBlendState, nullptr, D3D11_COLOR_WRITE_ENABLE_ALL);

    return true;
}