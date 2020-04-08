#pragma once

#include <Windows.h>
#include <d3d11.h>
#include <wrl/client.h>

class Display
{
public:
    // aspectRatio - width/height
    Display(HINSTANCE hInstance, unsigned int clientHeight, float aspectRatio, bool windowed);
    ~Display();

    /*
        Creates window and directx device and swap chain
    */

    ID3D11Device* getDevice();
    ID3D11DeviceContext* getDeviceContext();
    IDXGISwapChain* getSwapChain();
    ID3D11RenderTargetView* getRenderTarget();
    ID3D11DepthStencilView* getDepthStencilView();
    HWND getWindow();

    void showWindow();
    void presentBackBuffer();
    void clearBackBuffer();

    unsigned int getClientWidth() const     { return m_ClientWidth; }
    unsigned int getClientHeight() const    { return m_ClientHeight; }

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

    Microsoft::WRL::ComPtr<ID3D11Device> device;
    Microsoft::WRL::ComPtr<ID3D11DeviceContext> deviceContext;
    Microsoft::WRL::ComPtr<IDXGISwapChain> swapChain;
    Microsoft::WRL::ComPtr<ID3D11RenderTargetView> renderTarget;
    Microsoft::WRL::ComPtr<ID3D11DepthStencilState> depthStencilState;
    Microsoft::WRL::ComPtr<ID3D11Texture2D> depthStencilTx;
    Microsoft::WRL::ComPtr<ID3D11DepthStencilView> depthStencilView;
    Microsoft::WRL::ComPtr<ID3D11BlendState> mBlendState;

    // Color used to clear the back buffer
    FLOAT m_pClearColor[4] = {0.0f, 0.0f, 0.0f, 1.0f};

    bool windowed;

    const unsigned int m_FrameRateLimit = 60;
    unsigned int m_ClientWidth, m_ClientHeight;
    unsigned int m_WindowWidth, m_WindowHeight;

    // MSAA, 1 multisample means no 'extra samples'
    const unsigned int m_Multisamples = 1;
};
