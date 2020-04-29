#include "IGame.hpp"

#include <iostream>

IGame::IGame(HINSTANCE hInstance)
    :m_StateManager(&m_ECS),
    m_Display(hInstance, 720, 16.0f/9.0f, true),
    m_InputHandler(&m_ECS, m_Display.getWindow()),
    m_pPhysicsCore(nullptr),
    m_pAssetLoaders(nullptr),
    m_pUICore(nullptr),
    m_pRenderingCore(nullptr),
    m_pAudioCore(nullptr),
    m_RenderingHandler(&m_ECS, &m_Display)
{
    m_pPhysicsCore      = new PhysicsCore(&m_ECS);
    m_pAssetLoaders     = new AssetLoadersCore(&m_ECS, m_Display.getDevice());
    m_pUICore           = new UICore(&m_ECS, &m_Display);
    m_pRenderingCore    = new RenderingCore(&m_ECS, m_Display.getDevice());
    m_pAudioCore        = new AudioCore(&m_ECS);

    m_ECS.performRegistrations();
    m_Display.showWindow();
}

IGame::~IGame()
{
    delete m_pPhysicsCore;
    delete m_pAssetLoaders;
    delete m_pUICore;
    delete m_pRenderingCore;
    delete m_pAudioCore;
}

bool IGame::init()
{
    if (!m_RenderingHandler.init()) {
        return false;
    }

    return true;
}

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
            m_ECS.update(dt);
            m_StateManager.update(dt);
            m_pUICore->getButtonSystem().update(dt);

            m_RenderingHandler.render();
        }
    }
}
