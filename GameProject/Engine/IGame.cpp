#include "IGame.hpp"

#include <Engine/Rendering/Renderer.hpp>
#include <Engine/Rendering/ShaderHandler.hpp>

IGame::IGame()
{
    display.init(720, 16.0f/9.0f, true);

    shaderHandler = new ShaderHandler(display.getDevice(), &ecs.systemSubscriber);
    renderer = new Renderer(&ecs, display.getDevice());
}

IGame::~IGame()
{
    delete shaderHandler;
    delete renderer;
}
