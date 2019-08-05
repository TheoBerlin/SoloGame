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
        Transform& camTransform = transformHandler.transforms.indexID(camera);
        camTransform.position = {3.0f, 3.0f, 4.0f};

        DirectX::XMVECTOR camPos = DirectX::XMLoadFloat3(&camTransform.position);
        DirectX::XMVECTOR camLookDir = transformHandler.getForward(camTransform);
        DirectX::XMVECTOR camUpDir = {0.0f, 1.0f, 0.0f, 0.0f};
        vpHandler.createViewMatrix(camera, camPos, camLookDir, camUpDir);
        vpHandler.createProjMatrix(camera, 90.0f, 16.0f/9.0f, 0.1f, 20.0f);

        // Create renderable object
        renderableObject = ecs.entityIDGen.genID();
        transformHandler.createTransform(renderableObject);
        transformHandler.createWorldMatrix(renderableObject);
        renderableHandler.createRenderable(renderableObject, "./Game/Assets/Models/untitled.dae", PROGRAM::BASIC);
    }

    ecs.systemUpdater.updateMT(dt);
}
