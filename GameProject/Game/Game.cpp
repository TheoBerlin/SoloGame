#include "Game.hpp"

Game::Game(HINSTANCE hInstance)
    :IGame(hInstance),
    lightSpinner(&ecs),
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
        camTransform.position = {2.0f, 1.8f, 3.3f};

        DirectX::XMVECTOR camPos = DirectX::XMLoadFloat3(&camTransform.position);
        DirectX::XMVECTOR camLookDir = transformHandler.getForward(camTransform);
        DirectX::XMVECTOR camUpDir = {0.0f, 1.0f, 0.0f, 0.0f};
        vpHandler.createViewMatrix(camera, camPos, camLookDir, camUpDir);
        vpHandler.createProjMatrix(camera, 90.0f, 16.0f/9.0f, 0.1f, 20.0f);

        // Create renderable object
        renderableObject = ecs.entityIDGen.genID();
        transformHandler.createTransform(renderableObject);
        transformHandler.createWorldMatrix(renderableObject);
        renderableHandler.createRenderable(renderableObject, "./Game/Assets/Models/Cube.dae", PROGRAM::BASIC);

        // Create point light
        for (unsigned i = 0; i < MAX_POINTLIGHTS; i += 1) {
            Entity lightID = ecs.entityIDGen.genID();
            DirectX::XMFLOAT3 lightPos = {std::sinf(DirectX::XM_PIDIV2 * i) * 3.0f, 1.0f, std::cosf(DirectX::XM_PIDIV2 * i) * 3.0f};
            DirectX::XMFLOAT3 light = {std::sinf(DirectX::XM_PIDIV2*1.2f * i), 0.5f, std::cosf(DirectX::XM_PIDIV2*1.2f * i)};

            lightHandler.createPointLight(lightID, lightPos, light, 10.0f);
        }
    }

    ecs.systemUpdater.updateMT(dt);
}
