#include "IGame.hpp"

IGame::IGame()
    :display(720, 16.0f/9.0f, true),
    transformHandler(&ecs.systemSubscriber),
    shaderHandler(display.getDevice(), &ecs.systemSubscriber),
    txLoader(&ecs.systemSubscriber, display.getDevice()),
    modelLoader(&ecs.systemSubscriber, &txLoader, display.getDevice()),
    renderableHandler(&ecs.systemSubscriber),
    vpHandler(&ecs.systemSubscriber),
    lightHandler(&ecs.systemSubscriber),
    renderer(&ecs, display.getDevice(), display.getDeviceContext(), display.getRenderTarget(), display.getDepthStencilView())
{
    std::getchar();
}

IGame::~IGame()
{}
