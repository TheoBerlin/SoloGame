#include "GameSession.hpp"

#include <Engine/Audio/SoundHandler.hpp>
#include <Engine/ECS/ECSCore.hpp>
#include <Engine/Physics/Velocity.hpp>
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

    // Set mouse mode to relative and hide the cursor
    m_pInputHandler = static_cast<InputHandler*>(pComponentSubscriber->getComponentHandler(TID(InputHandler)));
    m_pInputHandler->setMouseMode(DirectX::Mouse::Mode::MODE_RELATIVE);
    m_pInputHandler->setMouseVisibility(false);

    TransformHandler* pTransformHandler = static_cast<TransformHandler*>(pComponentSubscriber->getComponentHandler(TID(TransformHandler)));
    RenderableHandler* pRenderableHandler = static_cast<RenderableHandler*>(pComponentSubscriber->getComponentHandler(TID(RenderableHandler)));
    SoundHandler* pSoundHandler = reinterpret_cast<SoundHandler*>(pComponentSubscriber->getComponentHandler(TID(SoundHandler)));

    const std::vector<DirectX::XMFLOAT3> sectionPoints = {
        {0.0f, 0.0f, 0.0f},
        {4.0f, 4.0f, -10.0f},
        {2.0f, 2.0f, -22.0f},
        {0.0f, 0.0f, -32.0f},
        {-3.0f, -6.0f, -42.0f},
    };

    for (const DirectX::XMFLOAT3& sectionPoint : sectionPoints) {
        createCube(sectionPoint, pSoundHandler, pTransformHandler, pRenderableHandler);
    }

    createPointLights(pSoundHandler, pTransformHandler, pComponentSubscriber);
    createTube(sectionPoints, pTransformHandler, pRenderableHandler);
    createPlayer(pTransformHandler, pComponentSubscriber);
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

void GameSession::createCube(const DirectX::XMFLOAT3& position, SoundHandler* pSoundHandler, TransformHandler* pTransformHandler, RenderableHandler* pRenderableHandler)
{
    Entity cube = m_pECS->createEntity();
    pTransformHandler->createTransform(cube, position, {0.5f, 0.5f, 0.5f});
    pTransformHandler->createWorldMatrix(cube);
    pRenderableHandler->createRenderable(cube, "./Game/Assets/Models/Cube.dae", PROGRAM::BASIC);

    // Attach sound to the cube
    const std::string soundFile = "./Game/Assets/Sounds/muscle-car-daniel_simon.mp3";
    if (pSoundHandler->createSound(cube, soundFile)) {
        pSoundHandler->playSound(cube);
    }
}

void GameSession::createPointLights(SoundHandler* pSoundHandler, TransformHandler* pTransformHandler, ComponentSubscriber* pComponentSubscriber)
{
    LightHandler* pLightHandler = static_cast<LightHandler*>(pComponentSubscriber->getComponentHandler(TID(LightHandler)));
    const std::string soundFile = "./Game/Assets/Sounds/muscle-car-daniel_simon.mp3";

    for (unsigned i = 0; i < 1u; i++) {
        Entity lightID = m_pECS->createEntity();
        DirectX::XMFLOAT3 lightPos  = {std::sinf(DirectX::XM_PIDIV2 * i) * 3.0f, 1.0f, std::cosf(DirectX::XM_PIDIV2 * i) * 3.0f};
        DirectX::XMFLOAT3 light     = {std::sinf(1.6f * i), 0.8f, std::cosf(1.2f * i)};

        pLightHandler->createPointLight(lightID, light, 10.0f);
        pTransformHandler->createPosition(lightID, lightPos);

        if (pSoundHandler->createSound(lightID, soundFile)) {
            pSoundHandler->playSound(lightID);
        }
    }
}

void GameSession::createTube(const std::vector<DirectX::XMFLOAT3>& sectionPoints, TransformHandler* pTransformHandler, RenderableHandler* pRenderableHandler)
{
    const float tubeRadius = 1.5f;
    const unsigned int tubeFaces = 10;
    Model* tubeModel = m_TubeHandler.createTube(sectionPoints, tubeRadius, tubeFaces);

    Entity tube = m_pECS->createEntity();
    pRenderableHandler->createRenderable(tube, tubeModel, PROGRAM::BASIC);
    pTransformHandler->createTransform(tube);
    pTransformHandler->createWorldMatrix(tube);
}

void GameSession::createPlayer(TransformHandler* pTransformHandler, ComponentSubscriber* pComponentSubscriber)
{
    Entity player = m_pECS->createEntity();

    const DirectX::XMFLOAT3 camPosition = {2.0f, 1.8f, 3.3f};
    pTransformHandler->createPosition(player, camPosition);
    pTransformHandler->createRotation(player);
    const DirectX::XMFLOAT4& camRotationQuat = pTransformHandler->getRotation(player);

    VelocityHandler* pVelocityHandler = reinterpret_cast<VelocityHandler*>(pComponentSubscriber->getComponentHandler(TID(VelocityHandler)));
    pVelocityHandler->createVelocityComponent(player);

    VPHandler* pVPHandler = static_cast<VPHandler*>(pComponentSubscriber->getComponentHandler(TID(VPHandler)));

    ViewMatrixInfo viewMatrixInfo = {};
    viewMatrixInfo.EyePosition      = DirectX::XMLoadFloat3(&camPosition);
    viewMatrixInfo.LookDirection    = pTransformHandler->getForward(camRotationQuat);
    viewMatrixInfo.UpDirection      = {0.0f, 1.0f, 0.0f, 0.0f};

    ProjectionMatrixInfo projMatrixInfo = {};
    projMatrixInfo.AspectRatio      = 16.0f/9.0f;
    projMatrixInfo.HorizontalFOV    = 90.0f;
    projMatrixInfo.NearZ            = 0.1f;
    projMatrixInfo.FarZ             = 20.0f;

    pVPHandler->createViewProjectionMatrices(player, viewMatrixInfo, projMatrixInfo);

    m_TrackPositionHandler.createTrackPosition(player);
}
