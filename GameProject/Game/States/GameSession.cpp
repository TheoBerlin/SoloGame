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
    m_TubeHandler(m_pECS, mainMenu->getDevice()),
    m_TrackPositionHandler(m_pECS),
    m_LightSpinner(m_pECS),
    m_RacerMover(m_pECS)
{
    m_pECS->performRegistrations();
    LOG_INFO("Started game session");

    ComponentSubscriber* pComponentSubscriber = m_pECS->getComponentSubscriber();

    // Set mouse mode to relative, which also hides the mouse
    this->m_pInputHandler = static_cast<InputHandler*>(pComponentSubscriber->getComponentHandler(TID(InputHandler)));
    m_pInputHandler->setMouseMode(DirectX::Mouse::Mode::MODE_RELATIVE);
    m_pInputHandler->setMouseVisibility(false);

    // Create renderable cube
    TransformHandler* transformHandler = static_cast<TransformHandler*>(pComponentSubscriber->getComponentHandler(TID(TransformHandler)));
    RenderableHandler* renderableHandler = static_cast<RenderableHandler*>(pComponentSubscriber->getComponentHandler(TID(RenderableHandler)));

    m_RenderableCube = m_pECS->createEntity();
    transformHandler->createTransform(m_RenderableCube);
    transformHandler->createWorldMatrix(m_RenderableCube);
    renderableHandler->createRenderable(m_RenderableCube, "./Game/Assets/Models/Cube.dae", PROGRAM::BASIC);

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
    const std::vector<DirectX::XMFLOAT3> sectionPoints = {
        {0.0f, 0.0f, 0.0f},
        {4.0f, 4.0f, -10.0f},
        {2.0f, 2.0f, -22.0f},
        {0.0f, 0.0f, -32.0f},
        {-3.0f, -6.0f, -42.0f},
    };

    const float tubeRadius = 1.5f;
    const unsigned int tubeFaces = 10;
    Model* tubeModel = m_TubeHandler.createTube(sectionPoints, tubeRadius, tubeFaces);

    Entity tube = m_pECS->createEntity();
    renderableHandler->createRenderable(tube, tubeModel, PROGRAM::BASIC);
    transformHandler->createTransform(tube);
    transformHandler->createWorldMatrix(tube);

    // Create camera
    m_Camera = m_pECS->createEntity();

    const DirectX::XMFLOAT3 camPosition = {2.0f, 1.8f, 3.3f};
    transformHandler->createPosition(m_Camera, camPosition);
    transformHandler->createRotation(m_Camera);
    const DirectX::XMFLOAT4& camRotationQuat = transformHandler->getRotation(m_Camera);

    VPHandler* vpHandler = static_cast<VPHandler*>(pComponentSubscriber->getComponentHandler(TID(VPHandler)));
    DirectX::XMVECTOR camPos     = DirectX::XMLoadFloat3(&camPosition);
    DirectX::XMVECTOR camLookDir = transformHandler->getForward(camRotationQuat);
    DirectX::XMVECTOR camUpDir   = {0.0f, 1.0f, 0.0f, 0.0f};
    vpHandler->createViewMatrix(m_Camera, camPos, camLookDir, camUpDir);
    vpHandler->createProjMatrix(m_Camera, 90.0f, 16.0f/9.0f, 0.1f, 20.0f);

    m_TrackPositionHandler.createTrackPosition(m_Camera);
}

GameSession::~GameSession()
{}

void GameSession::resume()
{
    m_pInputHandler->setMouseMode(DirectX::Mouse::Mode::MODE_RELATIVE);
    m_pInputHandler->setMouseVisibility(false);
}

void GameSession::pause()
{}

void GameSession::update(float dt)
{}
