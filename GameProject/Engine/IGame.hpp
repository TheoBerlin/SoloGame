#pragma once

#include <Engine/Audio/SoundHandler.hpp>
#include <Engine/Audio/SoundPlayer.hpp>
#include <Engine/ECS/ECSCore.hpp>
#include <Engine/GameState/StateManager.hpp>
#include <Engine/Rendering/AssetLoaders/ModelLoader.hpp>
#include <Engine/Rendering/AssetLoaders/TextureLoader.hpp>
#include <Engine/Rendering/Camera.hpp>
#include <Engine/Rendering/Components/PointLight.hpp>
#include <Engine/Rendering/Components/Renderable.hpp>
#include <Engine/Rendering/Components/VPMatrices.hpp>
#include <Engine/Rendering/Display.hpp>
#include <Engine/Rendering/RenderingHandler.hpp>
#include <Engine/Rendering/ShaderHandler.hpp>
#include <Engine/Rendering/ShaderResourceHandler.hpp>
#include <Engine/Rendering/Text/TextRenderer.hpp>
#include <Engine/UI/ButtonSystem.hpp>
#include <Engine/UI/Panel.hpp>
#include <Engine/InputHandler.hpp>
#include <Engine/Transform.hpp>

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

    // Component handlers
    TransformHandler m_TransformHandler;
    VPHandler m_VPHandler;
    Display m_Display;
    InputHandler m_InputHandler;
    ShaderHandler m_ShaderHandler;
    ShaderResourceHandler m_ShaderResourceHandler;
    TextureLoader m_TXLoader;
    ModelLoader m_ModelLoader;
    RenderableHandler m_RenderableHandler;
    UIHandler m_UIHandler;
    LightHandler m_LightHandler;
    TextRenderer m_TextRenderer;
    SoundHandler m_SoundHandler;

    // Systems
    CameraSystem m_CameraSystem;
    ButtonSystem m_ButtonSystem;
    SoundPlayer m_SoundPlayer;

    // Rendering
    RenderingHandler m_RenderingHandler;

    StateManager m_StateManager;
};
