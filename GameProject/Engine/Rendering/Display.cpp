#include "Display.hpp"

#include <Engine/Utils/Logger.hpp>
#include <combaseapi.h>
#include <comdef.h>
#include <roapi.h>

Display::Display()
    :hInstance(nullptr),
    hwnd(nullptr),
    device(nullptr),
    deviceContext(nullptr),
    windowed(true),
    swapChain(nullptr),
    width(0),
    height(0)
{
}

Display::~Display()
{
    if (this->device)
        this->device->Release();

    if (this->deviceContext)
        this->deviceContext->Release();

    if (this->swapChain)
        this->swapChain->Release();
}

bool Display::init(unsigned int height, float aspectRatio, bool windowed)
{
    this->height = height;
    this->width = (unsigned int)(height * aspectRatio);
    this->windowed = windowed;

    Logger::LOG_INFO("Creating window");

    if (!this->initWindow()) {
        return false;
    }

    return this->initDX();
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

bool Display::initWindow()
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
        return false;
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
        return false;
    }

    //ShowWindow(hwnd, SW_SHOW);

    // Allows for use of windows COM
    HRESULT hr = CoInitializeEx(nullptr, COINIT_MULTITHREADED);

    if (FAILED(hr)) {
        Logger::LOG_ERROR("Failed to initialize COM library");
        return false;
    }
    //Windows::Foundation::Initialize(RO_INIT_MULTITHREADED);


    return true;
}

bool Display::initDX()
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
    swapChainDesc.BufferDesc.Width = 0;
    swapChainDesc.BufferDesc.Height = 0;
    swapChainDesc.BufferDesc.RefreshRate.Numerator = frameRateLimit;
    swapChainDesc.BufferDesc.RefreshRate.Denominator = 0;
    swapChainDesc.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
    swapChainDesc.BufferDesc.ScanlineOrdering = DXGI_MODE_SCANLINE_ORDER_UNSPECIFIED;
    swapChainDesc.BufferDesc.Scaling = DXGI_MODE_SCALING_UNSPECIFIED;
    swapChainDesc.SampleDesc.Count = this->multisamples;
    swapChainDesc.SampleDesc.Quality = 0;
    swapChainDesc.BufferUsage = DXGI_USAGE_BACK_BUFFER;
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
        Logger::LOG_ERROR("Failed to create device and swap chain");
        return false;
    }

    return true;
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
