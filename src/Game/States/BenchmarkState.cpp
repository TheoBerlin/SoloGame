#include "BenchmarkState.hpp"

#include <Engine/ECS/ECSCore.hpp>
#include <Engine/InputHandler.hpp>
#include <Engine/Physics/Velocity.hpp>
#include <Engine/Rendering/AssetLoaders/AssetLoadersCore.hpp>
#include <Engine/Rendering/Components/PointLight.hpp>
#include <Engine/Rendering/Components/VPMatrices.hpp>
#include <Engine/Rendering/Window.hpp>
#include <Engine/Transform.hpp>
#include <Engine/Utils/RuntimeStats.hpp>

#include <Game/EntityCreators/TestEntityCreators.hpp>

#include <vendor/json/json.hpp>

#include <fstream>
#include <iomanip>

BenchmarkState::BenchmarkState(StateManager* pStateManager, const RuntimeStats* pRuntimeStats)
    :   State(pStateManager)
    ,   m_pRuntimeStats(pRuntimeStats)
    ,   m_RacerController(&m_TubeHandler)
{}

void BenchmarkState::Init()
{
    LOG_INFO("Started benchmark");

    EngineCore* pEngineCore = EngineCore::GetInstance();
    pEngineCore->GetRenderingCore()->GetWindow()->GetInputHandler()->disable();

    // Ambient sound
    AudioCore* pAudioCore = pEngineCore->GetAudioCore();
    const Entity ambientSoundEntity = ECSCore::GetInstance()->CreateEntity();
    constexpr const float ambientSoundVolume = 0.3f;
    pAudioCore->PlayLoopingSound(ambientSoundEntity, "./assets/Sounds/30306__erh__tension.wav", ambientSoundVolume);

    const std::vector<DirectX::XMFLOAT3> sectionPoints = {
        { 0.0f, 0.0f, 0.0f },
        { 4.0f, 4.0f, -10.0f },
        { 2.0f, 2.0f, -22.0f },
        { 0.0f, 0.0f, -32.0f },
        { -3.0f, -6.0f, -42.0f },
        { 1.0f, -9.0f, -52.0f },
        { -1.0f, -7.0f, -60.0f }
    };

    std::string soundPath = "./assets/Sounds/CakedInReverb1.wav";
    CreateMusicCubeEntity({ 1.0f, 0.0f, 0.0f }, soundPath);

    soundPath = "./assets/Sounds/MetalTrolly3.wav";
    CreateMusicCubeEntity({ 2.0f, 0.0f, 0.0f }, soundPath);

    soundPath = "./assets/Sounds/muscle-car-daniel_simon.mp3";
    for (const DirectX::XMFLOAT3& sectionPoint : sectionPoints) {
        CreateMusicCubeEntity(sectionPoint, soundPath);
    }

    CreatePointLights();
    CreateTube(sectionPoints);
    CreatePlayer();
}

void BenchmarkState::Resume()
{
    LOG_ERROR("Benchmark::resume was called. Benchmarks should be kept running continuously");
}

void BenchmarkState::Pause()
{
    LOG_ERROR("Benchmark::pause was called. Benchmarks should be kept running continuously");
}

void BenchmarkState::Update(float dt)
{
    UNREFERENCED_VARIABLE(dt);

    const TrackPositionComponent& trackPosition = ECSCore::GetInstance()->GetConstComponent<TrackPositionComponent>(m_PlayerEntity);
    if (trackPosition.section == m_TubeHandler.GetTubeSections().size() - 2 && trackPosition.T >= 1.0f) {
        // The end has been reached
        PrintBenchmarkResults();
        EngineCore::GetInstance()->GetRenderingCore()->GetWindow()->Close();
    }
}

void BenchmarkState::CreatePointLights()
{
    ECSCore* pECS = ECSCore::GetInstance();

    constexpr const float lightRadiusReciprocal = 1.0f / 10.0f;

    const std::string soundPath = "./assets/Sounds/muscle-car-daniel_simon.mp3";
    constexpr const float soundVolume = 1.0f;

    constexpr const uint32_t pointLightCount = 1;
    for (uint32_t i = 0; i < pointLightCount; i++) {
        const Entity lightEntity = pECS->CreateEntity();
        const DirectX::XMFLOAT3 lightPos  = { std::sinf(DirectX::XM_PIDIV2 * i) * 3.0f, 1.0f, std::cosf(DirectX::XM_PIDIV2 * i) * 3.0f };
        const DirectX::XMFLOAT3 light     = { std::sinf(1.6f * i), 0.8f, std::cosf(1.2f * i) };

        pECS->AddComponent(lightEntity, PositionComponent({ .Position = lightPos }));
        pECS->AddComponent(lightEntity, PointLightComponent({ .RadiusReciprocal = lightRadiusReciprocal, .Light = light }));

        EngineCore::GetInstance()->GetAudioCore()->PlayLoopingSound(lightEntity, soundPath, soundVolume);
    }
}

void BenchmarkState::CreateTube(const std::vector<DirectX::XMFLOAT3>& sectionPoints)
{
    constexpr const float tubeRadius = 1.5f;
    constexpr const uint32_t tubeFaces = 10;

    constexpr const DirectX::XMFLOAT3 tubePosition = DirectX::XMFLOAT3(0.0f, 0.0f, 0.0f);
    constexpr const DirectX::XMFLOAT3 tubeScale = DirectX::XMFLOAT3(1.0f, 1.0f, 1.0f);

    ECSCore* pECS = ECSCore::GetInstance();
    const Entity tubeEntity = pECS->CreateEntity();

    pECS->AddComponent(tubeEntity, m_TubeHandler.CreateTube(sectionPoints, tubeRadius, tubeFaces));

    pECS->AddComponent(tubeEntity, PositionComponent({ .Position = tubePosition }));
    pECS->AddComponent(tubeEntity, ScaleComponent({ .Scale = tubeScale }));
    pECS->AddComponent(tubeEntity, RotationComponent({ .Quaternion = g_QuaternionIdentity }));
    pECS->AddComponent(tubeEntity, WorldMatrixComponent({ .WorldMatrix = CreateWorldMatrix(tubePosition, tubeScale, g_QuaternionIdentity) }));
}

void BenchmarkState::CreatePlayer()
{
    ECSCore* pECS = ECSCore::GetInstance();
    m_PlayerEntity = pECS->CreateEntity();

    constexpr const DirectX::XMFLOAT3 camPosition = { 2.0f, 1.8f, 3.3f };
    pECS->AddComponent(m_PlayerEntity, PositionComponent({ .Position = camPosition }));
    pECS->AddComponent(m_PlayerEntity, RotationComponent({ .Quaternion = g_QuaternionIdentity }));
    pECS->AddComponent(m_PlayerEntity, VelocityComponent({ .Velocity = DirectX::XMFLOAT3(0.0f, 0.0f, 0.0f) }));

    const ViewMatrixInfo viewMatrixInfo = {
        .EyePosition    = DirectX::XMLoadFloat3(&camPosition),
        .LookDirection  = GetForward(g_QuaternionIdentity),
        .UpDirection    = g_DefaultUp
    };

    const ProjectionMatrixInfo projMatrixInfo = {
        .HorizontalFOV    = 90.0f,
        .AspectRatio      = 16.0f / 9.0f,
        .NearZ            = 0.1f,
        .FarZ             = 20.0f
    };

    const ViewProjectionMatricesComponent vpMatricesComp = CreateViewProjectionMatrices(viewMatrixInfo, projMatrixInfo);
    pECS->AddComponent(m_PlayerEntity, vpMatricesComp);

    // These will be initialized with proper values in a system subscribing to these types
    pECS->AddComponent(m_PlayerEntity, TrackPositionComponent({ }));
    pECS->AddComponent(m_PlayerEntity, TrackSpeedComponent({ }));
}

void BenchmarkState::PrintBenchmarkResults() const
{
    const char* pOutFile = "benchmark_results.json";
    LOG_INFOF("Writing benchmark results to %s", pOutFile);
    using json = nlohmann::json;

    constexpr const float MB = 1000000.0f;

    json benchmarkResults;
    benchmarkResults["AverageFPS"]      = 1.0f / m_pRuntimeStats->getAverageFrametime();
    benchmarkResults["PeakMemoryUsage"] = float(m_pRuntimeStats->getPeakMemoryUsage() / MB);

    std::ofstream benchmarkFile(pOutFile, std::fstream::out | std::fstream::trunc);
    benchmarkFile << std::setw(4) << benchmarkResults << std::endl;

    benchmarkFile.close();
}
