#include "Game.hpp"

Game::Game(HINSTANCE hInstance)
    :IGame(hInstance),
    hasSetup(false)
{}

Game::~Game()
{
    ecs.entityIDGen.popID(camera);
}

void Game::update(float dt)
{
    if (!hasSetup) {
        hasSetup = true;

        // Create camera
        camera = ecs.entityIDGen.genID();
        transformHandler.createTransform(camera);

        DirectX::XMVECTOR camPos = {0.0f, 0.0f, 0.0f, 0.0f};
        DirectX::XMVECTOR camLookDir = {0.0f, 0.0f, 1.0f, 0.0f};
        DirectX::XMVECTOR camUpDir = {0.0f, 1.0f, 0.0f, 0.0f};
        vpHandler.createViewMatrix(camera, camPos, camLookDir, camUpDir);
        vpHandler.createProjMatrix(camera, 90.0f, 16.0f/9.0f, 0.1f, 20.0f);

        // Create renderable object
        /*renderableObject = ecs.entityIDGen.genID();
        transformHandler.createTransform(renderableObject);

        DirectX::XMVECTOR camPos = {0.0f, 0.0f, 0.0f, 0.0f};
        DirectX::XMVECTOR camLookDir = {0.0f, 0.0f, 1.0f, 0.0f};
        DirectX::XMVECTOR camUpDir = {0.0f, 1.0f, 0.0f, 0.0f};
        vpHandler.createViewMatrix(renderableObject, camPos, camLookDir, camUpDir);
        vpHandler.createProjMatrix(renderableObject, 90.0f, 16.0f/9.0f, 0.1f, 20.0f);*/
    }

    ecs.systemUpdater.updateMT(dt);
}
