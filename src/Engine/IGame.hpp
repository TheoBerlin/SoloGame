#pragma once

#include <Engine/Audio/AudioCore.hpp>
#include <Engine/ECS/ECSCore.hpp>
#include <Engine/GameState/StateManager.hpp>
#include <Engine/Physics/PhysicsCore.hpp>
#include <Engine/Rendering/AssetLoaders/AssetLoadersCore.hpp>
#include <Engine/Rendering/RenderingCore.hpp>
#include <Engine/Rendering/RenderingHandler.hpp>
#include <Engine/Rendering/Window.hpp>
#include <Engine/UI/UICore.hpp>

class IGame
{
public:
    IGame();
    ~IGame();

    bool init();
    virtual bool finalize() = 0;

    // Starts the main loop
    void run();

protected:
    Window m_Window;
    Device* m_pDevice;

    ECSCore m_ECS;

    StateManager m_StateManager;

    // Containers for Component Handlers and Systems
    PhysicsCore* m_pPhysicsCore;
    AssetLoadersCore* m_pAssetLoaders;
    UICore* m_pUICore;
    RenderingCore* m_pRenderingCore;
    AudioCore* m_pAudioCore;

    // Rendering
    RenderingHandler* m_pRenderingHandler;
};