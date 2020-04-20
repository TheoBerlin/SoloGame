#include "GameSession.hpp"

#include <Engine/Audio/SoundHandler.hpp>
#include <Engine/ECS/ECSCore.hpp>
#include <Engine/Rendering/Components/Renderable.hpp>
#include <Engine/Rendering/Components/VPMatrices.hpp>
#include <Engine/Rendering/MeshRenderer.hpp>
#include <Engine/Transform.hpp>
#include <Engine/Utils/ECSUtils.hpp>
#include <Engine/Utils/Logger.hpp>
#include <Game/States/MainMenu.hpp>

GameSession::GameSession(MainMenu* mainMenu)
    :State(mainMenu, STATE_TRANSITION::POP_AND_PUSH),
    tubeHandler(m_pECS, mainMenu->getDevice()),
    trackPositionHandler(m_pECS),
    lightSpinner(m_pECS),
    racerMover(m_pECS)
{
    m_pECS->performRegistrations();
    LOG_INFO("Started game session");

    ComponentSubscriber* pComponentSubscriber = m_pECS->getComponentSubscriber();

    // Set mouse mode to relative, which also hides the mouse
    this->inputHandler = static_cast<InputHandler*>(pComponentSubscriber->getComponentHandler(TID(InputHandler)));
    inputHandler->setMouseMode(DirectX::Mouse::Mode::MODE_RELATIVE);
    inputHandler->setMouseVisibility(false);

    // Create camera
    camera = m_pECS->createEntity();

    TransformHandler* transformHandler = static_cast<TransformHandler*>(pComponentSubscriber->getComponentHandler(TID(TransformHandler)));
    DirectX::XMFLOAT3 camPosition = {2.0f, 1.8f, 3.3f};
    transformHandler->createTransform(camera, camPosition);
    Transform camTransform  = transformHandler->getTransform(camera);

    VPHandler* vpHandler = static_cast<VPHandler*>(pComponentSubscriber->getComponentHandler(TID(VPHandler)));
    DirectX::XMVECTOR camPos     = DirectX::XMLoadFloat3(&camPosition);
    DirectX::XMVECTOR camLookDir = transformHandler->getForward(camTransform.RotationQuaternion);
    DirectX::XMVECTOR camUpDir   = {0.0f, 1.0f, 0.0f, 0.0f};
    vpHandler->createViewMatrix(camera, camPos, camLookDir, camUpDir);
    vpHandler->createProjMatrix(camera, 90.0f, 16.0f/9.0f, 0.1f, 20.0f);

    //trackPositionHandler.createTrackPosition(camera);

    // Create renderable object
    RenderableHandler* renderableHandler = static_cast<RenderableHandler*>(pComponentSubscriber->getComponentHandler(TID(RenderableHandler)));

    renderableObject = m_pECS->createEntity();
    transformHandler->createTransform(renderableObject);
    transformHandler->createWorldMatrix(renderableObject);
    renderableHandler->createRenderable(renderableObject, "./Game/Assets/Models/Cube.dae", PROGRAM::BASIC);

    // Create point lights and attach sound to them
    LightHandler* lightHandler = static_cast<LightHandler*>(pComponentSubscriber->getComponentHandler(TID(LightHandler)));
    const std::string soundFile = "./Game/Assets/Sounds/muscle-car-daniel_simon.mp3";

    SoundHandler* pSoundHandler = reinterpret_cast<SoundHandler*>(pComponentSubscriber->getComponentHandler(TID(SoundHandler)));

    for (unsigned i = 0; i < 1u; i++) {
        Entity lightID = m_pECS->createEntity();
        DirectX::XMFLOAT3 lightPos  = {std::sinf(DirectX::XM_PIDIV2 * i) * 3.0f, 1.0f, std::cosf(DirectX::XM_PIDIV2 * i) * 3.0f};
        DirectX::XMFLOAT3 light     = {std::sinf(1.6f * i), 0.8f, std::cosf(1.2f * i)};

        lightHandler->createPointLight(lightID, lightPos, light, 10.0f);

        if (pSoundHandler->createSound(lightID, soundFile)) {
            pSoundHandler->playSound(lightID);
        }
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

    Entity tube = m_pECS->createEntity();
    renderableHandler->createRenderable(tube, tubeModel, PROGRAM::BASIC);
    transformHandler->createTransform(tube);
    transformHandler->createWorldMatrix(tube);
}

GameSession::~GameSession()
{}

void GameSession::resume()
{
    inputHandler->setMouseMode(DirectX::Mouse::Mode::MODE_RELATIVE);
    inputHandler->setMouseVisibility(false);
}

void GameSession::pause()
{}

void GameSession::update(float dt)
{}
