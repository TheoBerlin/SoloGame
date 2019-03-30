#pragma once

#include <Windows.h>
#include <d3d11.h>

class Display
{
public:
    Display();
    ~Display();

    /*
        Creates window and directx device and swap chain
        aspectRatio - width/height
    */
    bool init(unsigned int height, float aspectRatio, bool windowed);

    ID3D11Device* getDevice();
    ID3D11DeviceContext* getDeviceContext();
    IDXGISwapChain* getSwapChain();

private:
    bool initWindow();
    bool initDX();

    static LRESULT CALLBACK windowProcedure(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam);

    void printDisplayInfo() const;

    HINSTANCE hInstance;
    HWND hwnd;

    ID3D11Device* device;
    ID3D11DeviceContext* deviceContext;
    IDXGISwapChain* swapChain;

    bool windowed;

    const unsigned int frameRateLimit = 60;
    unsigned int width, height;

    // MSAA, 1 multisample means no 'extra samples'
    const unsigned int multisamples = 1;
};
