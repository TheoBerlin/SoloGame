#include "Display.hpp"

#include <DirectXTK/Keyboard.h>
#include <DirectXTK/Mouse.h>
#include <Engine/Utils/DirectXUtils.hpp>
#include <Engine/Utils/Logger.hpp>
#include <combaseapi.h>
#include <comdef.h>
#include <roapi.h>

bool Display::keepRunning = true;

Display::Display(HINSTANCE hInstance, unsigned int clientHeight, float aspectRatio, bool windowed)
    :hInstance(hInstance),
    hwnd(nullptr),
    device(nullptr),
    deviceContext(nullptr),
    windowed(windowed),
    swapChain(nullptr),
    renderTarget(nullptr),
    clientHeight(clientHeight),
    clientWidth((unsigned int)(clientHeight * aspectRatio))
{
    Log_Info("Creating window");

    this->initWindow();
    this->initDX();
}

Display::~Display()
{}

ID3D11Device* Display::getDevice()
{
    return this->device.Get();
}

ID3D11DeviceContext* Display::getDeviceContext()
{
    return this->deviceContext.Get();
}

IDXGISwapChain* Display::getSwapChain()
{
    return this->swapChain.Get();
}

ID3D11RenderTargetView* Display::getRenderTarget()
{
    return this->renderTarget.Get();
}

ID3D11DepthStencilView* Display::getDepthStencilView()
{
    return this->depthStencilView.Get();
}

HWND Display::getWindow()
{
    return this->hwnd;
}

void Display::showWindow()
{
    ShowWindow(hwnd, SW_SHOW);
}

void Display::presentBackBuffer()
{
    HRESULT hr = swapChain->Present(0, 0);
    if (FAILED(hr))
        Log_Warning("Failed to present swap-chain buffer: %s", hresultToString(hr).c_str());
}

void Display::clearBackBuffer()
{
    deviceContext->ClearRenderTargetView(renderTarget.Get(), clearColor);
    deviceContext->ClearDepthStencilView(depthStencilView.Get(), D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0, 0);
}

unsigned int Display::getWindowWidth() const
{
    return clientWidth;
}

unsigned int Display::getWindowHeight() const
{
    return clientHeight;
}

void Display::initWindow()
{
    // Create window class
    WNDCLASSEX wcex = {0};
    wcex.cbSize = sizeof(WNDCLASSEX);
    wcex.style = CS_HREDRAW | CS_VREDRAW;
    wcex.lpfnWndProc = &Display::windowProcedure;
    wcex.hInstance = this->hInstance;
    wcex.lpszClassName = "Game Name";

    if (!RegisterClassEx(&wcex)) {
        Log_Error("Failed to register window class");
        system("pause");
        exit(1);
    }

    RECT clientArea = {0, 0, (LONG)this->clientWidth, (LONG)this->clientHeight};
    AdjustWindowRect(&clientArea, WS_OVERLAPPEDWINDOW, FALSE);
    windowWidth = clientArea.right - clientArea.left;
    windowHeight = clientArea.bottom - clientArea.top;

    this->hwnd = CreateWindow(
        "Game Name",
        "Game Name",
        WS_OVERLAPPEDWINDOW,
        CW_USEDEFAULT,
        CW_USEDEFAULT,
        this->windowWidth,
        this->windowHeight,
        nullptr,
        nullptr,
        this->hInstance,
        nullptr);

    if (!this->hwnd) {
        Log_Error("Failed to create window");
        system("pause");
        exit(1);
    }

    // Allows for use of windows COM
    HRESULT hr = CoInitializeEx(nullptr, COINIT_MULTITHREADED);

    if (FAILED(hr)) {
        Log_Error("Failed to initialize COM library: %s", hresultToString(hr).c_str());
        system("pause");
        exit(1);
    }
    //Windows::Foundation::Initialize(RO_INIT_MULTITHREADED);
}

void Display::initDX()
{
    // Single-threaded flag improves performance when D3D11 calls are only made on one thread
    UINT deviceFlags = D3D11_CREATE_DEVICE_SINGLETHREADED;

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
    swapChainDesc.BufferDesc.Width = clientWidth;
    swapChainDesc.BufferDesc.Height = clientHeight;
    swapChainDesc.BufferDesc.RefreshRate.Numerator = frameRateLimit;
    swapChainDesc.BufferDesc.RefreshRate.Denominator = 1;
    swapChainDesc.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
    swapChainDesc.BufferDesc.ScanlineOrdering = DXGI_MODE_SCANLINE_ORDER_UNSPECIFIED;
    swapChainDesc.BufferDesc.Scaling = DXGI_MODE_SCALING_UNSPECIFIED;
    swapChainDesc.SampleDesc.Count = this->multisamples;
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
        Log_Error("Failed to create device and swap chain: %s", hresultToString(hr).c_str());
        system("pause");
        exit(1);
    }

    ID3D11Texture2D* backBuffer = nullptr;
    hr = swapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), (LPVOID*)&backBuffer);
    if (FAILED(hr)) {
        Log_Error("Failed to retrieve swap chain's back buffer: %s", hresultToString(hr).c_str());
        system("pause");
        exit(1);
    }

    hr = device->CreateRenderTargetView(backBuffer, nullptr, &renderTarget);
    if (FAILED(hr)) {
        Log_Error("Failed to create render target: %s", hresultToString(hr).c_str());
        system("pause");
        exit(1);
    }
    backBuffer->Release();

    /* Depth stencil */
    D3D11_TEXTURE2D_DESC depthTxDesc;
    ZeroMemory(&depthTxDesc, sizeof(D3D11_TEXTURE2D_DESC));
    depthTxDesc.Width = clientWidth;
    depthTxDesc.Height = clientHeight;
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
        Log_Error("Failed to create depth stencil texture: %s", hresultToString(hr).c_str());
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
        Log_Error("Failed to create depth stencil state: %s", hresultToString(hr).c_str());
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
        Log_Error("Failed to create depth stencil view: %s", hresultToString(hr).c_str());
        system("pause");
        exit(1);
    }

    deviceContext->ClearDepthStencilView(depthStencilView.Get(), D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0, 0);

    /* Set viewport */
    D3D11_VIEWPORT viewPort;
    viewPort.TopLeftX = 0;
    viewPort.TopLeftY = 0;
    viewPort.Width = (float)clientWidth;
    viewPort.Height = (float)clientHeight;
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
        Log_Error("Failed create blend state: %s", hresultToString(hr).c_str());
        system("pause");
        exit(1);
    }

    deviceContext->OMSetBlendState(mBlendState.Get(), nullptr, D3D11_COLOR_WRITE_ENABLE_ALL);
}

LRESULT CALLBACK Display::windowProcedure(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    switch (message)
    {
        case WM_DESTROY:
            keepRunning = false;
            break;
        case WM_ACTIVATEAPP:
            DirectX::Mouse::ProcessMessage(message, wParam, lParam);
            DirectX::Keyboard::ProcessMessage(message, wParam, lParam);
        case WM_MOUSEMOVE:
        case WM_LBUTTONDOWN:
        case WM_LBUTTONUP:
        case WM_RBUTTONDOWN:
        case WM_RBUTTONUP:
        case WM_MBUTTONDOWN:
        case WM_MBUTTONUP:
        case WM_MOUSEWHEEL:
        case WM_XBUTTONDOWN:
        case WM_XBUTTONUP:
        case WM_MOUSEHOVER:
        case WM_INPUT:
            DirectX::Mouse::ProcessMessage(message, wParam, lParam);
            break;
        case WM_KEYDOWN:
        case WM_SYSKEYDOWN:
        case WM_KEYUP:
        case WM_SYSKEYUP:
            DirectX::Keyboard::ProcessMessage(message, wParam, lParam);
            break;
    }

    return DefWindowProc(hwnd, message, wParam, lParam);
}
