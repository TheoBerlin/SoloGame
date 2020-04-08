#include "IGame.hpp"

#include <iostream>

IGame::IGame(HINSTANCE hInstance)
    :m_StateManager(&m_ECS),
    m_Display(hInstance, 720, 16.0f/9.0f, true),
    m_InputHandler(&m_ECS, m_Display.getWindow()),
    m_TransformHandler(&m_ECS),
    m_ShaderHandler(m_Display.getDevice(), &m_ECS),
    m_ShaderResourceHandler(&m_ECS, m_Display.getDevice()),
    m_TXLoader(&m_ECS, m_Display.getDevice()),
    m_ModelLoader(&m_ECS, &m_TXLoader),
    m_RenderableHandler(&m_ECS),
    m_VPHandler(&m_ECS),
    m_LightHandler(&m_ECS),
    m_TextRenderer(&m_ECS, m_Display.getDevice(), m_Display.getDeviceContext()),
    m_UIHandler(&m_ECS, &m_Display),
    m_SoundHandler(&m_ECS),
    m_MeshRenderer(&m_ECS, m_Display.getDevice(), m_Display.getDeviceContext(), m_Display.getRenderTarget(), m_Display.getDepthStencilView()),
    m_UIRenderer(&m_ECS, m_Display.getDeviceContext(), m_Display.getDevice(), m_Display.getRenderTarget(), m_Display.getDepthStencilView()),
    m_CameraSystem(&m_ECS),
    m_ButtonSystem(&m_ECS, m_Display.getWindowWidth(), m_Display.getWindowHeight()),
    m_SoundPlayer(&m_ECS)
{
    m_ECS.performRegistrations();

    m_Display.showWindow();
    m_MeshRenderer.update(0.0f);
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

            m_InputHandler.update();

            // Update logic
            m_StateManager.update(dt);

            m_ButtonSystem.update(dt);

            // Render
            m_Display.clearBackBuffer();
            m_MeshRenderer.update(dt);
            m_UIRenderer.update(dt);
            m_Display.presentBackBuffer();

            m_ECS.performMaintenance();
        }
    }
}
