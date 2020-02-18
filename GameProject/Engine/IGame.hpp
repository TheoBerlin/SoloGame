#pragma once

#include <Engine/ECS/ECSCore.hpp>
#include <Engine/GameState/StateManager.hpp>
#include <Engine/Rendering/AssetLoaders/ModelLoader.hpp>
#include <Engine/Rendering/AssetLoaders/TextureLoader.hpp>
#include <Engine/Rendering/Camera.hpp>
#include <Engine/Rendering/Components/PointLight.hpp>
#include <Engine/Rendering/Components/Renderable.hpp>
#include <Engine/Rendering/Components/VPMatrices.hpp>
#include <Engine/Rendering/Display.hpp>
#include <Engine/Rendering/Renderer.hpp>
#include <Engine/Rendering/ShaderHandler.hpp>
#include <Engine/Rendering/ShaderResourceHandler.hpp>
#include <Engine/Rendering/Text/TextRenderer.hpp>
#include <Engine/UI/ButtonSystem.hpp>
#include <Engine/UI/Panel.hpp>
#include <Engine/UI/UIRenderer.hpp>
#include <Engine/InputHandler.hpp>
#include <Engine/Transform.hpp>

class IGame
{
public:
    IGame(HINSTANCE hInstance);
    ~IGame();

    // Starts the main loop
    void run();

protected:
    ECSCore m_ECS;

    // Component handlers
    TransformHandler transformHandler;
    VPHandler vpHandler;
    Display display;
    InputHandler inputHandler;
    ShaderHandler shaderHandler;
    ShaderResourceHandler shaderResourceHandler;
    TextureLoader txLoader;
    ModelLoader modelLoader;
    RenderableHandler renderableHandler;
    UIHandler uiHandler;
    LightHandler lightHandler;
    TextRenderer textRenderer;

    // Systems
    Renderer renderer;
    UIRenderer uiRenderer;
    CameraSystem cameraSystem;
    ButtonSystem buttonSystem;

    StateManager stateManager;
};
