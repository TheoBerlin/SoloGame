#pragma once

#include <Engine/ECS/ECSInterface.hpp>
#include <Engine/Rendering/Display.hpp>
#include <Engine/Rendering/ShaderHandler.hpp>

class Renderer;
class ShaderHandler;

class IGame
{
public:
    IGame();
    ~IGame();

private:
    ECSInterface ecs;
    Display display;
    ShaderHandler* shaderHandler;
    Renderer* renderer;
};
