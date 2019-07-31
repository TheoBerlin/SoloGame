#include "Display.hpp"

#include <Engine/Utils/DirectXUtils.hpp>
#include <Engine/Utils/Logger.hpp>
#include <combaseapi.h>
#include <comdef.h>
#include <roapi.h>

Display::Display(unsigned int height, float aspectRatio, bool windowed)
    :hInstance(nullptr),
    hwnd(nullptr),
    device(nullptr),
    deviceContext(nullptr),
    windowed(windowed),
    swapChain(nullptr),
    renderTarget(nullptr),
    height(height),
    width((unsigned int)(height * aspectRatio))
{
    Logger::LOG_INFO("Creating window");

    this->initWindow();
    this->initDX();
}

Display::~Display()
{
    if (this->device)
        this->device->Release();

    if (this->deviceContext)
        this->deviceContext->Release();

    if (this->swapChain)
        this->swapChain->Release();

    if (this->renderTarget)
        this->renderTarget->Release();

    if (this->depthStencilState)
        this->depthStencilState->Release();

    if (this->depthStencilTx)
        this->depthStencilTx->Release();

    if (this->depthStencilView)
        this->depthStencilView->Release();
}

ID3D11Device* Display::getDevice()
{
    return this->device;
}

ID3D11DeviceContext* Display::getDeviceContext()
{
    return this->deviceContext;
}

IDXGISwapChain* Display::getSwapChain()
{
    return this->swapChain;
}

ID3D11RenderTargetView* Display::getRenderTarget()
{
    return this->renderTarget;
}

ID3D11DepthStencilView* Display::getDepthStencilView()
{
    return this->depthStencilView;
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
        Logger::LOG_ERROR("Failed to register window class");
        system("pause");
        exit(1);
    }

    RECT clientArea = {0, 0, (LONG)this->width, (LONG)this->height};
    AdjustWindowRect(&clientArea, WS_OVERLAPPEDWINDOW, FALSE);

    this->hwnd = CreateWindow(
        "Game Name",
        "Game Name",
        WS_OVERLAPPEDWINDOW,
        CW_USEDEFAULT,
        CW_USEDEFAULT,
        this->width,
        this->height,
        nullptr,
        nullptr,
        this->hInstance,
        nullptr);

    if (!this->hwnd) {
        Logger::LOG_ERROR("Failed to create window");
        system("pause");
        exit(1);
    }

    //ShowWindow(hwnd, SW_SHOW);

    // Allows for use of windows COM
    HRESULT hr = CoInitializeEx(nullptr, COINIT_MULTITHREADED);

    if (FAILED(hr)) {
        Logger::LOG_ERROR("Failed to initialize COM library: %s", hresultToString(hr).c_str());
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
    swapChainDesc.BufferDesc.Width = 0; // Will 'automatically' be adjusted to the size of the window
    swapChainDesc.BufferDesc.Height = 0;
    swapChainDesc.BufferDesc.RefreshRate.Numerator = frameRateLimit;
    swapChainDesc.BufferDesc.RefreshRate.Denominator = 1;
    swapChainDesc.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
    swapChainDesc.BufferDesc.ScanlineOrdering = DXGI_MODE_SCANLINE_ORDER_UNSPECIFIED;
    swapChainDesc.BufferDesc.Scaling = DXGI_MODE_SCALING_UNSPECIFIED;
    swapChainDesc.SampleDesc.Count = this->multisamples;
    swapChainDesc.SampleDesc.Quality = 0;
    swapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT | DXGI_USAGE_UNORDERED_ACCESS;
    swapChainDesc.BufferCount = 1;
    swapChainDesc.OutputWindow = this->hwnd;
    swapChainDesc.Windowed = this->windowed;

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
        &this->swapChain,
        &this->device,
        &createdFeatureLevel,
        &this->deviceContext);

    if (FAILED(hr) || !device || !swapChain) {
        Logger::LOG_ERROR("Failed to create device and swap chain: %s", hresultToString(hr).c_str());
        system("pause");
        exit(1);
    }

    ID3D11Texture2D* backBuffer = nullptr;
    hr = swapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), (LPVOID*)&backBuffer);
    if (FAILED(hr)) {
        Logger::LOG_ERROR("Failed to retrieve swap chain's back buffer: %s", hresultToString(hr).c_str());
        system("pause");
        exit(1);
    }

    hr = device->CreateRenderTargetView(backBuffer, nullptr, &renderTarget);
    if (FAILED(hr)) {
        Logger::LOG_ERROR("Failed to create render target: %s", hresultToString(hr).c_str());
        system("pause");
        exit(1);
    }

    /* Depth stencil */
    D3D11_TEXTURE2D_DESC depthTxDesc;
    ZeroMemory(&depthTxDesc, sizeof(D3D11_TEXTURE2D_DESC));
    depthTxDesc.Width = width;
    depthTxDesc.Height = height;
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
        Logger::LOG_ERROR("Failed to create depth stencil texture: %s", hresultToString(hr).c_str());
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
        Logger::LOG_ERROR("Failed to create depth stencil state: %s", hresultToString(hr).c_str());
        system("pause");
        exit(1);
    }

    D3D11_DEPTH_STENCIL_VIEW_DESC dsViewDesc;
    dsViewDesc.Format = DXGI_FORMAT_D32_FLOAT;
    dsViewDesc.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D;
    dsViewDesc.Flags = 0;
    dsViewDesc.Texture2D.MipSlice = 0;

    hr = device->CreateDepthStencilView(depthStencilTx, &dsViewDesc, &depthStencilView);
    if (FAILED(hr)) {
        Logger::LOG_ERROR("Failed to create depth stencil view: %s", hresultToString(hr).c_str());
        system("pause");
        exit(1);
    }

    backBuffer->Release();
}

LRESULT CALLBACK Display::windowProcedure(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    switch (message)
    {
        case WM_DESTROY:
            PostQuitMessage(0);
            break;
    }

    return DefWindowProc(hwnd, message, wParam, lParam);
}
