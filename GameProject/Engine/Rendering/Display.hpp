#pragma once

#include <Windows.h>
#include <d3d11.h>

class Display
{
public:
    // aspectRatio - width/height
    Display(HINSTANCE hInstance, unsigned int height, float aspectRatio, bool windowed);
    ~Display();

    /*
        Creates window and directx device and swap chain
    */

    ID3D11Device* getDevice();
    ID3D11DeviceContext* getDeviceContext();
    IDXGISwapChain* getSwapChain();
    ID3D11RenderTargetView* getRenderTarget();
    ID3D11DepthStencilView* getDepthStencilView();

    void showWindow();
    void presentBackBuffer();

    // Set to false when window is being closed (eg. alt+f4)
    static bool keepRunning;

private:
    // Creates window
    void initWindow();
    // Creates DirectX device, device context, swap chain, render target and depth stencil
    void initDX();

    static LRESULT CALLBACK windowProcedure(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam);

    void printDisplayInfo() const;

    HINSTANCE hInstance;
    HWND hwnd;

    ID3D11Device* device;
    ID3D11DeviceContext* deviceContext;
    IDXGISwapChain* swapChain;
    ID3D11RenderTargetView* renderTarget;
    ID3D11DepthStencilState* depthStencilState;
    ID3D11Texture2D* depthStencilTx;
    ID3D11DepthStencilView* depthStencilView;

    bool windowed;

    const unsigned int frameRateLimit = 60;
    unsigned int width, height;

    // MSAA, 1 multisample means no 'extra samples'
    const unsigned int multisamples = 1;
};
