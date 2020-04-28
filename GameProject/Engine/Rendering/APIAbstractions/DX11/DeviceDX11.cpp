#include "DeviceDX11.hpp"

#include <d3d11.h>

DeviceDX11::DeviceDX11()
{}

DeviceDX11::~DeviceDX11()
{}

bool DeviceDX11::init(const SwapChainInfo& swapChainInfo)
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
    swapChainDesc.BufferDesc.Width = swapChainInfo.BackBufferRes.x;
    swapChainDesc.BufferDesc.Height = swapChainInfo.BackBufferRes.y;
    swapChainDesc.BufferDesc.RefreshRate.Numerator = (UINT)swapChainInfo.FrameRateLimit;
    swapChainDesc.BufferDesc.RefreshRate.Denominator = 1;
    swapChainDesc.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
    swapChainDesc.BufferDesc.ScanlineOrdering = DXGI_MODE_SCANLINE_ORDER_UNSPECIFIED;
    swapChainDesc.BufferDesc.Scaling = DXGI_MODE_SCALING_UNSPECIFIED;
    swapChainDesc.SampleDesc.Count = (UINT)swapChainInfo.Multisamples;
    swapChainDesc.SampleDesc.Quality = 0;
    swapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT | DXGI_USAGE_UNORDERED_ACCESS;
    swapChainDesc.BufferCount = 2;
    swapChainDesc.OutputWindow = this->hwnd;
    swapChainDesc.Windowed = this->windowed;
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
        this->swapChain.GetAddressOf(),
        this->device.GetAddressOf(),
        &createdFeatureLevel,
        this->deviceContext.GetAddressOf());

    if (FAILED(hr) || !device || !swapChain) {
        LOG_ERROR("Failed to create device and swap chain: %s", hresultToString(hr).c_str());
        system("pause");
        exit(1);
    }

    ID3D11Texture2D* backBuffer = nullptr;
    hr = swapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), (LPVOID*)&backBuffer);
    if (FAILED(hr)) {
        LOG_ERROR("Failed to retrieve swap chain's back buffer: %s", hresultToString(hr).c_str());
        system("pause");
        exit(1);
    }

    hr = device->CreateRenderTargetView(backBuffer, nullptr, &renderTarget);
    if (FAILED(hr)) {
        LOG_ERROR("Failed to create render target: %s", hresultToString(hr).c_str());
        system("pause");
        exit(1);
    }
    backBuffer->Release();

    /* Depth stencil */
    D3D11_TEXTURE2D_DESC depthTxDesc;
    ZeroMemory(&depthTxDesc, sizeof(D3D11_TEXTURE2D_DESC));
    depthTxDesc.Width = m_ClientWidth;
    depthTxDesc.Height = m_ClientHeight;
    depthTxDesc.MipLevels = 1;
    depthTxDesc.ArraySize = 1;
    depthTxDesc.Format = DXGI_FORMAT_D32_FLOAT;
    depthTxDesc.SampleDesc.Count = 1;
    depthTxDesc.SampleDesc.Quality = 0;
    depthTxDesc.Usage = D3D11_USAGE_DEFAULT;
    depthTxDesc.BindFlags = D3D11_BIND_DEPTH_STENCIL;
    depthTxDesc.CPUAccessFlags = 0;
    depthTxDesc.MiscFlags = 0;

    hr = device->CreateTexture2D(&depthTxDesc, nullptr, &depthStencilTx);
    if (FAILED(hr)) {
        LOG_ERROR("Failed to create depth stencil texture: %s", hresultToString(hr).c_str());
        system("pause");
        exit(1);
    }

    D3D11_DEPTH_STENCIL_DESC dsDesc;
    ZeroMemory(&dsDesc, sizeof(D3D11_DEPTH_STENCIL_DESC));
    dsDesc.DepthEnable = true;
    dsDesc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ALL;
    dsDesc.DepthFunc = D3D11_COMPARISON_LESS;
    dsDesc.StencilEnable = false;
    dsDesc.FrontFace.StencilDepthFailOp = D3D11_STENCIL_OP_KEEP;
    dsDesc.FrontFace.StencilPassOp = D3D11_STENCIL_OP_KEEP;
    dsDesc.FrontFace.StencilFunc = D3D11_COMPARISON_LESS;
    dsDesc.BackFace = dsDesc.FrontFace;

    hr = device->CreateDepthStencilState(&dsDesc, &depthStencilState);
    if (FAILED(hr)) {
        LOG_ERROR("Failed to create depth stencil state: %s", hresultToString(hr).c_str());
        system("pause");
        exit(1);
    }

    D3D11_DEPTH_STENCIL_VIEW_DESC dsViewDesc;
    ZeroMemory(&dsViewDesc, sizeof(D3D11_DEPTH_STENCIL_VIEW_DESC));
    dsViewDesc.Format = DXGI_FORMAT_D32_FLOAT;
    dsViewDesc.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D;
    dsViewDesc.Flags = 0;
    dsViewDesc.Texture2D.MipSlice = 0;

    hr = device->CreateDepthStencilView(depthStencilTx.Get(), &dsViewDesc, &depthStencilView);
    if (FAILED(hr)) {
        LOG_ERROR("Failed to create depth stencil view: %s", hresultToString(hr).c_str());
        system("pause");
        exit(1);
    }

    deviceContext->ClearDepthStencilView(depthStencilView.Get(), D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0, 0);

    /* Set viewport */
    D3D11_VIEWPORT viewPort;
    viewPort.TopLeftX = 0;
    viewPort.TopLeftY = 0;
    viewPort.Width = (float)m_ClientWidth;
    viewPort.Height = (float)m_ClientHeight;
    viewPort.MinDepth = 0.0f;
    viewPort.MaxDepth = 1.0f;
    deviceContext->RSSetViewports(1, &viewPort);

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

    hr = device->CreateBlendState(&blendDesc, mBlendState.GetAddressOf());
    if (hr != S_OK) {
        LOG_ERROR("Failed create blend state: %s", hresultToString(hr).c_str());
        system("pause");
        exit(1);
    }

    deviceContext->OMSetBlendState(mBlendState.Get(), nullptr, D3D11_COLOR_WRITE_ENABLE_ALL);
}
