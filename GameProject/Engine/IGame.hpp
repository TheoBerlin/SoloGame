#pragma once

#include <Engine/ECS/ECSInterface.hpp>
#include <Engine/Rendering/AssetLoaders/ModelLoader.hpp>
#include <Engine/Rendering/AssetLoaders/TextureLoader.hpp>
#include <Engine/Rendering/Camera.hpp>
#include <Engine/Rendering/Components/PointLight.hpp>
#include <Engine/Rendering/Components/Renderable.hpp>
#include <Engine/Rendering/Components/VPMatrices.hpp>
#include <Engine/Rendering/Display.hpp>
#include <Engine/Rendering/Renderer.hpp>
#include <Engine/Rendering/ShaderHandler.hpp>
#include <Engine/InputHandler.hpp>
#include <Engine/Transform.hpp>

class IGame
{
public:
    IGame(HINSTANCE hInstance);
    ~IGame();

    // Starts the main loop
    void run();

    virtual void update(float dt) = 0;

protected:
    ECSInterface ecs;
    TransformHandler transformHandler;
    VPHandler vpHandler;
    Display display;
    InputHandler inputHandler;
    ShaderHandler shaderHandler;
    TextureLoader txLoader;
    ModelLoader modelLoader;
    RenderableHandler renderableHandler;
    LightHandler lightHandler;
    Renderer renderer;
    CameraSystem cameraSystem;
};
