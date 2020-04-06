#include "IGame.hpp"

#include <iostream>

IGame::IGame(HINSTANCE hInstance)
    :stateManager(&m_ECS),
    display(hInstance, 720, 16.0f/9.0f, true),
    inputHandler(&m_ECS, display.getWindow()),
    transformHandler(&m_ECS),
    shaderHandler(display.getDevice(), &m_ECS),
    shaderResourceHandler(&m_ECS, display.getDevice()),
    txLoader(&m_ECS, display.getDevice()),
    modelLoader(&m_ECS, &txLoader),
    renderableHandler(&m_ECS),
    vpHandler(&m_ECS),
    lightHandler(&m_ECS),
    textRenderer(&m_ECS, display.getDevice(), display.getDeviceContext()),
    uiHandler(&m_ECS, &display),
    renderer(&m_ECS, display.getDevice(), display.getDeviceContext(), display.getRenderTarget(), display.getDepthStencilView()),
    uiRenderer(&m_ECS, display.getDeviceContext(), display.getDevice(), display.getRenderTarget(), display.getDepthStencilView()),
    cameraSystem(&m_ECS),
    buttonSystem(&m_ECS, display.getWindowWidth(), display.getWindowHeight())
{
    m_ECS.performRegistrations();

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
    while(WM_QUIT != msg.message) {
        if (!Display::keepRunning) {
            break;
        }

        if (PeekMessage(&msg, nullptr, 0, 0, PM_REMOVE)) {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        } else {
            timeNow = std::chrono::high_resolution_clock::now();
            dtChrono = timeNow - timer;
            float dt = dtChrono.count();

            timer = timeNow;

            inputHandler.update();

            // Update logic
            stateManager.update(dt);

            buttonSystem.update(dt);

            // Render
            display.clearBackBuffer();
            renderer.update(dt);
            uiRenderer.update(dt);
            display.presentBackBuffer();

            m_ECS.performMaintenance();
        }
    }
}
