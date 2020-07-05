#include "IGame.hpp"

#include <Engine/Utils/Debug.hpp>

#include <iostream>

IGame::IGame()
    :m_Window(720u, 16.0f / 9.0f, true),
    m_pDevice(nullptr),
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
    delete m_pDevice;
}

bool IGame::init()
{
    if (!m_Window.init()) {
        return false;
    }

    SwapchainInfo swapChainInfo = {};
    swapChainInfo.FrameRateLimit    = 60u;
    swapChainInfo.Multisamples      = 1u;
    swapChainInfo.Windowed          = true;

    DescriptorCounts descriptorPoolSize;
    descriptorPoolSize.setAll(100u);

    m_pDevice = Device::create(RENDERING_API::DIRECTX11, swapChainInfo, &m_Window);
    if (!m_pDevice || !m_pDevice->init(descriptorPoolSize)) {
        return false;
    }

    m_pPhysicsCore      = DBG_NEW PhysicsCore(&m_ECS);
    m_pAssetLoaders     = DBG_NEW AssetLoadersCore(&m_ECS, m_pDevice);
    m_pUICore           = DBG_NEW UICore(&m_ECS, m_pDevice, &m_Window);
    m_pRenderingCore    = DBG_NEW RenderingCore(&m_ECS, m_pDevice, &m_Window);
    m_pAudioCore        = DBG_NEW AudioCore(&m_ECS);

    m_pRenderingHandler = DBG_NEW RenderingHandler(&m_ECS, m_pDevice);

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
