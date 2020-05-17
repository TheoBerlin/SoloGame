#include "IGame.hpp"

#include <Engine/Rendering/APIAbstractions/DX11/DeviceDX11.hpp>

#include <iostream>

IGame::IGame()
    :m_Window(720u, 16.0f / 9.0f, true),
    m_StateManager(&m_ECS),
    m_pPhysicsCore(nullptr),
    m_pAssetLoaders(nullptr),
    m_pUICore(nullptr),
    m_pRenderingCore(nullptr),
    m_pAudioCore(nullptr),
    m_pRenderingHandler(nullptr)
{}

IGame::~IGame()
{
    delete m_pPhysicsCore;
    delete m_pAssetLoaders;
    delete m_pUICore;
    delete m_pRenderingCore;
    delete m_pAudioCore;

    delete m_pRenderingHandler;
}

bool IGame::init()
{
    if (!m_Window.init()) {
        return false;
    }

    SwapChainInfo swapChainInfo = {};
    swapChainInfo.FrameRateLimit    = 60u;
    swapChainInfo.Multisamples      = 1;
    swapChainInfo.Windowed          = true;

    DescriptorCounts descriptorPoolSize;
    descriptorPoolSize.setAll(100u);

    if (!m_Device.init(swapChainInfo, &m_Window, descriptorPoolSize)) {
        return false;
    }

    m_pPhysicsCore      = new PhysicsCore(&m_ECS);
    m_pAssetLoaders     = new AssetLoadersCore(&m_ECS, &m_Device);
    m_pUICore           = new UICore(&m_ECS, &m_Device, &m_Window);
    m_pRenderingCore    = new RenderingCore(&m_ECS, &m_Device, &m_Window);
    m_pAudioCore        = new AudioCore(&m_ECS);

    m_pRenderingHandler = new RenderingHandler(&m_ECS, &m_Device, &m_Window);

    m_ECS.performRegistrations();
    m_Window.show();

    if (!m_pRenderingHandler->init()) {
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
    while(!m_Window.shouldClose()) {
        m_Window.pollEvents();

        timeNow = std::chrono::high_resolution_clock::now();
        dtChrono = timeNow - timer;
        float dt = dtChrono.count();

        timer = timeNow;

        m_Window.getInputHandler()->update();

        // Update logic
        m_ECS.update(dt);
        m_StateManager.update(dt);
        m_pUICore->getButtonSystem().update(dt);

        m_pRenderingHandler->render();
    }
}
