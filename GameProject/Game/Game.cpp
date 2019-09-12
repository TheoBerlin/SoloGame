#include "Game.hpp"

Game::Game(HINSTANCE hInstance)
    :IGame(hInstance),
    lightSpinner(&ecs),
    tubeHandler(&ecs.systemSubscriber, display.getDevice()),
    trackPositionHandler(&ecs.systemSubscriber),
    racerMover(&ecs),
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
        DirectX::XMVECTOR camLookDir = transformHandler.getForward(camTransform.rotQuat);
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
            DirectX::XMFLOAT3 light = {std::sinf(1.6f * i), 0.8f, std::cosf(1.2f * i)};

            lightHandler.createPointLight(lightID, lightPos, light, 10.0f);
        }

        // Create tube
        std::vector<DirectX::XMFLOAT3> sectionPoints = {
                {0.0f, 0.0f, 0.0f},
                {4.0f, 4.0f, -10.0f},
                {2.0f, 2.0f, -22.0f},
                {0.0f, 0.0f, -32.0f},
                {-3.0f, -6.0f, -42.0f},
        };

        const float tubeRadius = 1.5f;
        const unsigned int tubeFaces = 10;
        Model* tubeModel = tubeHandler.createTube(sectionPoints, tubeRadius, tubeFaces);

        Entity tube = ecs.entityIDGen.genID();
        renderableHandler.createRenderable(tube, tubeModel, PROGRAM::BASIC);
        transformHandler.createTransform(tube);
        transformHandler.createWorldMatrix(tube);

        trackPositionHandler.createTrackPosition(camera);
    }

    ecs.systemUpdater.updateMT(dt);
}
