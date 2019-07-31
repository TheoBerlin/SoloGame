#pragma once

#include <Engine/ECS/ECSInterface.hpp>
#include <Engine/Rendering/AssetLoaders/ModelLoader.hpp>
#include <Engine/Rendering/AssetLoaders/TextureLoader.hpp>
#include <Engine/Rendering/Components/PointLight.hpp>
#include <Engine/Rendering/Components/Renderable.hpp>
#include <Engine/Rendering/Components/VPMatrices.hpp>
#include <Engine/Rendering/Display.hpp>
#include <Engine/Rendering/Renderer.hpp>
#include <Engine/Rendering/ShaderHandler.hpp>
#include <Engine/Transform.hpp>

class IGame
{
public:
    IGame();
    ~IGame();

private:
    ECSInterface ecs;
    Display display;
    TransformHandler transformHandler;
    ShaderHandler shaderHandler;
    TextureLoader txLoader;
    ModelLoader modelLoader;
    RenderableHandler renderableHandler;
    VPHandler vpHandler;
    LightHandler lightHandler;
    Renderer renderer;
};
