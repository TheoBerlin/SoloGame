#include "Benchmark.hpp"

#include <Engine/Audio/SoundHandler.hpp>
#include <Engine/ECS/ECSCore.hpp>
#include <Engine/InputHandler.hpp>
#include <Engine/Physics/Velocity.hpp>
#include <Engine/Rendering/AssetLoaders/ModelLoader.hpp>
#include <Engine/Rendering/Components/PointLight.hpp>
#include <Engine/Rendering/Components/VPMatrices.hpp>
#include <Engine/Rendering/Window.hpp>
#include <Engine/Transform.hpp>
#include <Engine/Utils/RuntimeStats.hpp>

#include <vendor/json/json.hpp>

#include <fstream>
#include <iomanip>

Benchmark::Benchmark(StateManager* pStateManager, ECSCore* pECS, Device* pDevice, InputHandler* pInputHandler, const RuntimeStats* pRuntimeStats, Window* pWindow)
    :State(pStateManager, pECS),
    m_pInputHandler(pInputHandler),
    m_pRuntimeStats(pRuntimeStats),
    m_pWindow(pWindow),
    m_pDevice(pDevice),
    m_TrackPositionHandler(m_pECS),
    m_TubeHandler(m_pECS, pDevice),
    m_LightSpinner(m_pECS),
    m_RacerController(m_pECS, pInputHandler, &m_TubeHandler)
{}

void Benchmark::init()
{
    LOG_INFO("Started benchmark");

    m_pInputHandler->disable();
    m_pECS->performRegistrations();

    ComponentSubscriber* pComponentSubscriber   = m_pECS->getComponentSubscriber();
    ModelLoader* pModelLoader                   = reinterpret_cast<ModelLoader*>(pComponentSubscriber->getComponentHandler(TID(ModelLoader)));
    TransformHandler* pTransformHandler         = reinterpret_cast<TransformHandler*>(pComponentSubscriber->getComponentHandler(TID(TransformHandler)));
    SoundHandler* pSoundHandler                 = reinterpret_cast<SoundHandler*>(pComponentSubscriber->getComponentHandler(TID(SoundHandler)));

    startMusic(pSoundHandler);

    const std::vector<DirectX::XMFLOAT3> sectionPoints = {
        {0.0f, 0.0f, 0.0f},
        {4.0f, 4.0f, -10.0f},
        {2.0f, 2.0f, -22.0f},
        {0.0f, 0.0f, -32.0f},
        {-3.0f, -6.0f, -42.0f},
        {1.0f, -9.0f, -52.0f},
        {-1.0f, -7.0f, -60.0f}
    };

    std::string soundPath = "./assets/Sounds/CakedInReverb1.wav";
    createCube({1.0f, 0.0f, 0.0f}, soundPath, pSoundHandler, pTransformHandler, pModelLoader);

    soundPath = "./assets/Sounds/MetalTrolly3.wav";
    createCube({2.0f, 0.0f, 0.0f}, soundPath, pSoundHandler, pTransformHandler, pModelLoader);

    soundPath = "./assets/Sounds/muscle-car-daniel_simon.mp3";
    for (const DirectX::XMFLOAT3& sectionPoint : sectionPoints) {
        createCube(sectionPoint, soundPath, pSoundHandler, pTransformHandler, pModelLoader);
    }

    createPointLights(pSoundHandler, pTransformHandler, pComponentSubscriber);
    createTube(sectionPoints, pTransformHandler, pModelLoader);
    createPlayer(pTransformHandler, pComponentSubscriber);
}

void Benchmark::resume()
{
    LOG_ERROR("Benchmark::resume was called. Benchmarks should be kept running continuously");
}

void Benchmark::pause()
{
    LOG_ERROR("Benchmark::pause was called. Benchmarks should be kept running continuously");
}

void Benchmark::update(float dt)
{
    const TrackPosition& trackPosition = m_TrackPositionHandler.getTrackPosition(m_PlayerEntity);
    if (trackPosition.section == m_TubeHandler.getTubeSections().size() - 2 && trackPosition.T >= 1.0f) {
        // The end has been reached
        printBenchmarkResults();
        m_pWindow->close();
    }
}

void Benchmark::startMusic(SoundHandler* pSoundHandler)
{
    const float musicVolume = 0.3f;

    Entity music = m_pECS->createEntity();
    if (pSoundHandler->createSound(music, "./assets/Sounds/30306__erh__tension.wav")) {
        pSoundHandler->loopSound(music);
        pSoundHandler->playSound(music);
        pSoundHandler->setVolume(music, musicVolume);
    }
}

void Benchmark::createCube(const DirectX::XMFLOAT3& position, const std::string& soundPath, SoundHandler* pSoundHandler, TransformHandler* pTransformHandler, ModelLoader* pModelLoader)
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

void Benchmark::createPointLights(SoundHandler* pSoundHandler, TransformHandler* pTransformHandler, ComponentSubscriber* pComponentSubscriber)
{
    LightHandler* pLightHandler = reinterpret_cast<LightHandler*>(pComponentSubscriber->getComponentHandler(TID(LightHandler)));
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

void Benchmark::createTube(const std::vector<DirectX::XMFLOAT3>& sectionPoints, TransformHandler* pTransformHandler, ModelLoader* pModelLoader)
{
    const float tubeRadius = 1.5f;
    const unsigned int tubeFaces = 10;
    Model* pTubeModel = m_TubeHandler.createTube(sectionPoints, tubeRadius, tubeFaces);

    Entity tube = m_pECS->createEntity();
    pModelLoader->registerModel(tube, pTubeModel);
    pTransformHandler->createTransform(tube);
    pTransformHandler->createWorldMatrix(tube);
}

void Benchmark::createPlayer(TransformHandler* pTransformHandler, ComponentSubscriber* pComponentSubscriber)
{
    m_PlayerEntity = m_pECS->createEntity();

    const DirectX::XMFLOAT3 camPosition = {2.0f, 1.8f, 3.3f};
    pTransformHandler->createPosition(m_PlayerEntity, camPosition);
    pTransformHandler->createRotation(m_PlayerEntity);
    const DirectX::XMFLOAT4& camRotationQuat = pTransformHandler->getRotation(m_PlayerEntity);

    VelocityHandler* pVelocityHandler = reinterpret_cast<VelocityHandler*>(pComponentSubscriber->getComponentHandler(TID(VelocityHandler)));
    pVelocityHandler->createVelocityComponent(m_PlayerEntity);

    VPHandler* pVPHandler = reinterpret_cast<VPHandler*>(pComponentSubscriber->getComponentHandler(TID(VPHandler)));

    ViewMatrixInfo viewMatrixInfo = {};
    viewMatrixInfo.EyePosition      = DirectX::XMLoadFloat3(&camPosition);
    viewMatrixInfo.LookDirection    = pTransformHandler->getForward(camRotationQuat);
    viewMatrixInfo.UpDirection      = {0.0f, 1.0f, 0.0f, 0.0f};

    ProjectionMatrixInfo projMatrixInfo = {};
    projMatrixInfo.AspectRatio      = 16.0f/9.0f;
    projMatrixInfo.HorizontalFOV    = 90.0f;
    projMatrixInfo.NearZ            = 0.1f;
    projMatrixInfo.FarZ             = 20.0f;

    pVPHandler->createViewProjectionMatrices(m_PlayerEntity, viewMatrixInfo, projMatrixInfo);

    m_TrackPositionHandler.createTrackPosition(m_PlayerEntity);
    m_TrackPositionHandler.createTrackSpeed(m_PlayerEntity);
}

void Benchmark::printBenchmarkResults() const
{
    const char* outFile = "benchmark_results.json";
    LOG_INFOF("Writing benchmark results to %s", outFile);
    using json = nlohmann::json;

    json benchmarkResults;
    benchmarkResults["AverageFPS"] = 1.0f / m_pRuntimeStats->getAverageFrametime();

    std::ofstream benchmarkFile(outFile, std::fstream::out | std::fstream::trunc);
    benchmarkFile << std::setw(4) << benchmarkResults << std::endl;

    benchmarkFile.close();
}
