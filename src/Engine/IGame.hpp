#pragma once

#include <Engine/Audio/AudioCore.hpp>
#include <Engine/ECS/ECSCore.hpp>
#include <Engine/EngineCore.hpp>
#include <Engine/GameState/StateManager.hpp>
#include <Engine/Physics/PhysicsCore.hpp>
#include <Engine/Rendering/AssetLoaders/AssetLoadersCore.hpp>
#include <Engine/Rendering/RenderingCore.hpp>
#include <Engine/Rendering/RenderingHandler.hpp>
#include <Engine/UI/UICore.hpp>
#include <Engine/Utils/RuntimeStats.hpp>

namespace argh {
    class parser;
}

struct EngineConfig {
    RENDERING_API RenderingAPI;
    PRESENTATION_MODE PresentationMode;
};

class IGame
{
public:
    IGame();
    virtual ~IGame();

    bool Init();
    virtual bool Finalize(const argh::parser& flagParser) = 0;

    // Starts the main loop
    void Run();

protected:
    ECSCore m_ECS;

    StateManager m_StateManager;

    RuntimeStats m_RuntimeStats;

    EngineCore m_EngineCore;

    // Rendering
    RenderingHandler* m_pRenderingHandler;
};
