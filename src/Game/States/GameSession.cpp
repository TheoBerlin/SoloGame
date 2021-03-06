#include "GameSession.hpp"

#include <Engine/Audio/SoundHandler.hpp>
#include <Engine/ECS/ECSCore.hpp>
#include <Engine/InputHandler.hpp>
#include <Engine/Physics/Velocity.hpp>
#include <Engine/Rendering/AssetLoaders/ModelLoader.hpp>
#include <Engine/Rendering/Components/PointLight.hpp>
#include <Engine/Rendering/Components/VPMatrices.hpp>
#include <Engine/Transform.hpp>
#include <Engine/Utils/ECSUtils.hpp>
#include <Game/States/MainMenu.hpp>

GameSession::GameSession(MainMenu* pMainMenu)
    :State(pMainMenu),
    m_pInputHandler(pMainMenu->getInputHandler()),
    m_TubeHandler(m_pECS, pMainMenu->getDevice()),
    m_TrackPositionHandler(m_pECS),
    m_LightSpinner(m_pECS),
    m_RacerController(m_pECS, pMainMenu->getInputHandler(), &m_TubeHandler)
{}

void GameSession::init()
{
    LOG_INFO("Started game session");

    m_pECS->performRegistrations();

    // Set mouse mode to relative and hide the cursor
    m_pInputHandler->hideCursor();

    EntityPublisher* pEntityPublisher   = m_pECS->getEntityPublisher();
    ModelLoader* pModelLoader                   = reinterpret_cast<ModelLoader*>(pEntityPublisher->getComponentHandler(TID(ModelLoader)));
    TransformHandler* pTransformHandler         = reinterpret_cast<TransformHandler*>(pEntityPublisher->getComponentHandler(TID(TransformHandler)));
    SoundHandler* pSoundHandler                 = reinterpret_cast<SoundHandler*>(pEntityPublisher->getComponentHandler(TID(SoundHandler)));

    startMusic(pSoundHandler);

    const std::vector<DirectX::XMFLOAT3> sectionPoints = {
        {0.0f, 0.0f, 0.0f},
        {4.0f, 4.0f, -10.0f},
        {2.0f, 2.0f, -22.0f},
        {0.0f, 0.0f, -32.0f},
        {-3.0f, -6.0f, -42.0f},
    };

    std::string soundPath = "./assets/Sounds/CakedInReverb1.wav";
    createCube({1.0f, 0.0f, 0.0f}, soundPath, pSoundHandler, pTransformHandler, pModelLoader);

    soundPath = "./assets/Sounds/MetalTrolly3.wav";
    createCube({2.0f, 0.0f, 0.0f}, soundPath, pSoundHandler, pTransformHandler, pModelLoader);

    soundPath = "./assets/Sounds/muscle-car-daniel_simon.mp3";
    for (const DirectX::XMFLOAT3& sectionPoint : sectionPoints) {
        createCube(sectionPoint, soundPath, pSoundHandler, pTransformHandler, pModelLoader);
    }

    createPointLights(pSoundHandler, pTransformHandler, pEntityPublisher);
    createTube(sectionPoints, pTransformHandler, pModelLoader);
    createPlayer(pTransformHandler, pEntityPublisher);
}

void GameSession::resume()
{
    m_pInputHandler->hideCursor();
}

void GameSession::pause()
{}

void GameSession::update(float dt)
{
    UNREFERENCED_VARIABLE(dt);
}

void GameSession::startMusic(SoundHandler* pSoundHandler)
{
    const float musicVolume = 0.3f;

    Entity music = m_pECS->createEntity();
    if (pSoundHandler->createSound(music, "./assets/Sounds/30306__erh__tension.wav")) {
        pSoundHandler->loopSound(music);
        pSoundHandler->playSound(music);
        pSoundHandler->setVolume(music, musicVolume);
    }
}

void GameSession::createCube(const DirectX::XMFLOAT3& position, const std::string& soundPath, SoundHandler* pSoundHandler, TransformHandler* pTransformHandler, ModelLoader* pModelLoader)
{
    Entity cube = m_pECS->createEntity();
    pTransformHandler->createTransform(cube, position, {0.5f, 0.5f, 0.5f});
    pTransformHandler->createWorldMatrix(cube);
    pModelLoader->loadModel(cube, "./assets/Models/Cube.dae");

    // Attach sound to the cube
    if (pSoundHandler->createSound(cube, soundPath)) {
        pSoundHandler->loopSound(cube);
        pSoundHandler->playSound(cube);
    }
}

void GameSession::createPointLights(SoundHandler* pSoundHandler, TransformHandler* pTransformHandler, EntityPublisher* pEntityPublisher)
{
    LightHandler* pLightHandler = reinterpret_cast<LightHandler*>(pEntityPublisher->getComponentHandler(TID(LightHandler)));
    const std::string soundFile = "./assets/Sounds/muscle-car-daniel_simon.mp3";

    for (unsigned i = 0; i < 1u; i++) {
        Entity lightID = m_pECS->createEntity();
        DirectX::XMFLOAT3 lightPos  = {std::sinf(DirectX::XM_PIDIV2 * i) * 3.0f, 1.0f, std::cosf(DirectX::XM_PIDIV2 * i) * 3.0f};
        DirectX::XMFLOAT3 light     = {std::sinf(1.6f * i), 0.8f, std::cosf(1.2f * i)};

        pLightHandler->createPointLight(lightID, light, 10.0f);
        pTransformHandler->createPosition(lightID, lightPos);

        if (pSoundHandler->createSound(lightID, soundFile)) {
            pSoundHandler->loopSound(lightID);
            pSoundHandler->playSound(lightID);
        }
    }
}

void GameSession::createTube(const std::vector<DirectX::XMFLOAT3>& sectionPoints, TransformHandler* pTransformHandler, ModelLoader* pModelLoader)
{
    const float tubeRadius = 1.5f;
    const unsigned int tubeFaces = 10;
    Model* pTubeModel = m_TubeHandler.createTube(sectionPoints, tubeRadius, tubeFaces);

    Entity tube = m_pECS->createEntity();
    pModelLoader->registerModel(tube, pTubeModel);
    pTransformHandler->createTransform(tube);
    pTransformHandler->createWorldMatrix(tube);
}

void GameSession::createPlayer(TransformHandler* pTransformHandler, EntityPublisher* pEntityPublisher)
{
    Entity player = m_pECS->createEntity();

    const DirectX::XMFLOAT3 camPosition = {2.0f, 1.8f, 3.3f};
    pTransformHandler->createPosition(player, camPosition);
    pTransformHandler->createRotation(player);
    const DirectX::XMFLOAT4& camRotationQuat = pTransformHandler->getRotation(player);

    VelocityHandler* pVelocityHandler = reinterpret_cast<VelocityHandler*>(pEntityPublisher->getComponentHandler(TID(VelocityHandler)));
    pVelocityHandler->createVelocityComponent(player);

    VPHandler* pVPHandler = reinterpret_cast<VPHandler*>(pEntityPublisher->getComponentHandler(TID(VPHandler)));

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
}
