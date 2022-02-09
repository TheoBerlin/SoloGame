#include "IGame.hpp"

#include <Engine/Utils/Debug.hpp>

IGame::IGame()
    :m_pRenderingHandler(nullptr)
{
    m_ECS.SetInstance(&m_ECS);
}

IGame::~IGame()
{
    Device* pDevice = m_EngineCore.GetRenderingCore()->GetDevice();
    if (pDevice) {
        pDevice->waitIdle();
    }

    m_StateManager.Release();

    delete m_pRenderingHandler;
}

bool IGame::Init()
{
    EngineCore::SetInstance(&m_EngineCore);
    if (!m_EngineCore.Init()) {
        return false;
    }

    RenderingCore* pRenderingCore = m_EngineCore.GetRenderingCore();
    m_pRenderingHandler = DBG_NEW RenderingHandler(m_EngineCore.GetRenderingCore());
    if (!m_pRenderingHandler->Init()) {
        return false;
    }

    pRenderingCore->GetWindow()->show();

    return true;
}

void IGame::Run()
{
    auto timer = std::chrono::high_resolution_clock::now();
    auto timeNow = timer;
    std::chrono::duration<float> dtChrono;

    Window* pWindow = m_EngineCore.GetRenderingCore()->GetWindow();

    while (!pWindow->shouldClose()) {
        pWindow->pollEvents();

        timeNow = std::chrono::high_resolution_clock::now();
        dtChrono = timeNow - timer;
        const float dt = dtChrono.count();

        timer = timeNow;
        m_RuntimeStats.setFrameTime(dt);

        pWindow->GetInputHandler()->update();

        // Update logic
        m_ECS.Update(dt);
        m_StateManager.Update(dt);

        m_pRenderingHandler->render();
    }
}
