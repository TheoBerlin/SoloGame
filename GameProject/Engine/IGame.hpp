#pragma once

#include <Engine/Audio/AudioCore.hpp>
#include <Engine/ECS/ECSCore.hpp>
#include <Engine/GameState/StateManager.hpp>
#include <Engine/Physics/PhysicsCore.hpp>
#include <Engine/Rendering/AssetLoaders/AssetLoadersCore.hpp>
#include <Engine/Rendering/RenderingCore.hpp>
#include <Engine/Rendering/Display.hpp>
#include <Engine/Rendering/RenderingHandler.hpp>
#include <Engine/UI/UICore.hpp>
#include <Engine/InputHandler.hpp>

class IGame
{
public:
    IGame(HINSTANCE hInstance);
    ~IGame();

    bool init();

    // Starts the main loop
    void run();

protected:
    ECSCore m_ECS;
    Display m_Display;
    InputHandler m_InputHandler;

    StateManager m_StateManager;

    // Containers for Component Handlers and Systems
    PhysicsCore* m_pPhysicsCore;
    AssetLoadersCore* m_pAssetLoaders;
    UICore* m_pUICore;
    RenderingCore* m_pRenderingCore;
    AudioCore* m_pAudioCore;

    // Rendering
    RenderingHandler m_RenderingHandler;
};
