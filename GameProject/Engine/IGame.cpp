#include "IGame.hpp"

IGame::IGame(HINSTANCE hInstance)
    :display(hInstance, 720, 16.0f/9.0f, true),
    transformHandler(&ecs.systemSubscriber),
    shaderHandler(display.getDevice(), &ecs.systemSubscriber),
    txLoader(&ecs.systemSubscriber, display.getDevice()),
    modelLoader(&ecs.systemSubscriber, &txLoader, display.getDevice()),
    renderableHandler(&ecs.systemSubscriber),
    vpHandler(&ecs.systemSubscriber),
    lightHandler(&ecs.systemSubscriber),
    renderer(&ecs, display.getDevice(), display.getDeviceContext(), display.getRenderTarget(), display.getDepthStencilView()),
    cameraSystem(&ecs)
{
    display.showWindow();
    renderer.update(0.0f);
}

IGame::~IGame()
{}

void IGame::run()
{
    auto timer = std::chrono::high_resolution_clock::now();
    auto timeNow = timer;
    std::chrono::duration<float> dtChrono;


    MSG msg = {0};
    while(WM_QUIT != msg.message)
    { 
        if (!Display::keepRunning)
        {
            break;
        }
        if (PeekMessage(&msg, nullptr, 0, 0, PM_REMOVE))
        {
            TranslateMessage(&msg); 
            DispatchMessage(&msg); 
        } else {
            timeNow = std::chrono::high_resolution_clock::now();
            dtChrono = timeNow-timer;
            float dt = dtChrono.count();

            timer = timeNow;

            this->update(dt);
            renderer.update(dt);
            display.presentBackBuffer();
        }
    }
}
