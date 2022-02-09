#include "GameSession.hpp"

#include <Engine/Audio/SoundPlayer.hpp>
#include <Engine/ECS/ECSCore.hpp>
#include <Engine/InputHandler.hpp>
#include <Engine/Physics/Velocity.hpp>
#include <Engine/Rendering/AssetLoaders/ModelLoader.hpp>
#include <Engine/Rendering/Camera.hpp>
#include <Engine/Rendering/Components/PointLight.hpp>
#include <Engine/Rendering/Components/VPMatrices.hpp>
#include <Engine/Transform.hpp>
#include <Engine/Utils/ECSUtils.hpp>
#include <Game/EntityCreators/TestEntityCreators.hpp>
#include <Game/States/MainMenuState.hpp>

GameSession::GameSession(MainMenuState* pMainMenu)
    :State(pMainMenu),
    m_TubeHandler(),
    m_LightSpinner(),
    m_RacerController(&m_TubeHandler)
{}

void GameSession::Init()
{
    LOG_INFO("Started game session");

    // Set mouse mode to relative and hide the cursor
    EngineCore* pEngineCore = EngineCore::GetInstance();
    InputHandler* pInputHandler = pEngineCore->GetRenderingCore()->GetWindow()->GetInputHandler();
    pInputHandler->hideCursor();

    // Play ambient sound
    ECSCore* pECS = ECSCore::GetInstance();
    const Entity ambientSoundEntity = pECS->CreateEntity();

    constexpr const float ambientSoundVolume = 0.3f;
    std::string soundPath = "./assets/Sounds/CakedInReverb1.wav";

    pEngineCore->GetAudioCore()->PlayLoopingSound(ambientSoundEntity, soundPath, ambientSoundVolume);

    soundPath = "./assets/Sounds/CakedInReverb1.wav";
    CreateMusicCubeEntity({ 1.0f, 0.0f, 0.0f }, soundPath);

    soundPath = "./assets/Sounds/MetalTrolly3.wav";
    CreateMusicCubeEntity({ 2.0f, 0.0f, 0.0f }, soundPath);

    // Create tube
    const std::vector<DirectX::XMFLOAT3> sectionPoints = {
        {0.0f, 0.0f, 0.0f},
        {4.0f, 4.0f, -10.0f},
        {2.0f, 2.0f, -22.0f},
        {0.0f, 0.0f, -32.0f},
        {-3.0f, -6.0f, -42.0f},
    };

    soundPath = "./assets/Sounds/muscle-car-daniel_simon.mp3";
    for (const DirectX::XMFLOAT3& sectionPoint : sectionPoints) {
        CreateMusicCubeEntity(sectionPoint, soundPath);
    }

    CreatePointLights();
    CreateTube(sectionPoints);
    CreatePlayer();
}

void GameSession::Resume()
{
    EngineCore* pEngineCore = EngineCore::GetInstance();
    InputHandler* pInputHandler = pEngineCore->GetRenderingCore()->GetWindow()->GetInputHandler();
    pInputHandler->hideCursor();
}

void GameSession::Pause()
{}

void GameSession::Update(float dt)
{
    UNREFERENCED_VARIABLE(dt);
}

void GameSession::CreatePointLights()
{
    const std::string soundPath = "./assets/Sounds/muscle-car-daniel_simon.mp3";

    for (unsigned i = 0; i < 1u; i++) {
        const DirectX::XMFLOAT3 lightPos    = { std::sinf(DirectX::XM_PIDIV2 * i) * 3.0f, 1.0f, std::cosf(DirectX::XM_PIDIV2 * i) * 3.0f };
        DirectX::XMFLOAT3 light             = { std::sinf(1.6f * i), 0.8f, std::cosf(1.2f * i) };

        CreateMusicPointLightEntity(lightPos, light, soundPath);
    }
}

void GameSession::CreateTube(const std::vector<DirectX::XMFLOAT3>& sectionPoints)
{
    constexpr const float tubeRadius          = 1.5f;
    constexpr const unsigned int tubeFaces    = 10;
    const ModelComponent tubeModel  = m_TubeHandler.CreateTube(sectionPoints, tubeRadius, tubeFaces);

    ECSCore* pECS       = ECSCore::GetInstance();
    const Entity tube   = pECS->CreateEntity();
    pECS->AddComponent<ModelComponent>(tube, tubeModel);

    constexpr const DirectX::XMFLOAT3 pos       = { 0.0f, 0.0f, 0.0f };
    constexpr const DirectX::XMFLOAT3 scale     = { 1.0f, 1.0f, 1.0f };
    constexpr const DirectX::XMFLOAT4 rotation  = g_QuaternionIdentity;

    pECS->AddComponent<PositionComponent>(tube, { .Position = pos });
    pECS->AddComponent<ScaleComponent>(tube, { .Scale = scale });
    pECS->AddComponent<RotationComponent>(tube, { .Quaternion = rotation });
    pECS->AddComponent<WorldMatrixComponent>(tube, { .WorldMatrix = CreateWorldMatrix(pos, scale, rotation) });
}

void GameSession::CreatePlayer()
{
    ECSCore* pECS = ECSCore::GetInstance();
    const Entity player = pECS->CreateEntity();

    constexpr const DirectX::XMFLOAT3 pos       = { 2.0f, 1.8f, 3.3f };
    constexpr const DirectX::XMFLOAT4 rotation  = g_QuaternionIdentity;

    const ViewMatrixInfo viewMatrixInfo = {
        .EyePosition    = DirectX::XMLoadFloat3(&pos),
        .LookDirection  = GetForward(rotation),
        .UpDirection    = g_DefaultUp
    };

    const ProjectionMatrixInfo projMatrixInfo = {
        .HorizontalFOV  = 90.0f,
        .AspectRatio    = 16.0f / 9.0f,
        .NearZ          = 0.1f,
        .FarZ           = 20.0f
    };

    pECS->AddComponent<PositionComponent>(player, { .Position = pos });
    pECS->AddComponent<RotationComponent>(player, { .Quaternion = rotation });
    pECS->AddComponent<VelocityComponent>(player, { .Velocity = {} });
    pECS->AddComponent<ViewProjectionMatricesComponent>(player, CreateViewProjectionMatrices(viewMatrixInfo, projMatrixInfo));
    pECS->AddComponent<CameraTagComponent>(player, {});
}
