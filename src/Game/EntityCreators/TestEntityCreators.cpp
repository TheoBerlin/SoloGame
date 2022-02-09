#include "TestEntityCreators.hpp"

#include <Engine/Transform.hpp>

Entity CreateMusicCubeEntity(const DirectX::XMFLOAT3& position, const std::string& soundPath)
{
    constexpr const DirectX::XMFLOAT3 scale = DirectX::XMFLOAT3(0.5f, 0.5f, 0.5f);

    ECSCore* pECS = ECSCore::GetInstance();
    const Entity cubeEntity = pECS->CreateEntity();
    pECS->AddComponent(cubeEntity, PositionComponent({ .Position = position }));
    pECS->AddComponent(cubeEntity, ScaleComponent({ .Scale = scale  }));
    pECS->AddComponent(cubeEntity, RotationComponent({ .Quaternion = g_QuaternionIdentity }));
    pECS->AddComponent(cubeEntity, WorldMatrixComponent({ .WorldMatrix = CreateWorldMatrix(position, scale, g_QuaternionIdentity) }));

    EngineCore* pEngineCore = EngineCore::GetInstance();
    AssetLoadersCore* pAssetLoaders = pEngineCore->GetAssetLoadersCore();
    pECS->AddComponent(cubeEntity, pAssetLoaders->GetModelLoader()->LoadModel("./assets/Models/Cube.dae"));

    // Attach sound to the cube
    AudioCore* pAudioCore = pEngineCore->GetAudioCore();
    constexpr const float soundVolume = 1.0f;
    pAudioCore->PlayLoopingSound(cubeEntity, soundPath, soundVolume);

    return cubeEntity;
}

Entity CreateMusicPointLightEntity(const DirectX::XMFLOAT3& position, const DirectX::XMFLOAT3& light, const std::string& soundPath)
{
    ECSCore* pECS = ECSCore::GetInstance();
    const Entity entity = pECS->CreateEntity();
    pECS->AddComponent(entity, PositionComponent({ .Position = position }));
    pECS->AddComponent(entity, PointLightComponent({ .RadiusReciprocal = 1.0f / 10.0f, .Light = light }));

    EngineCore* pEngineCore = EngineCore::GetInstance();
    AssetLoadersCore* pAssetLoaders = pEngineCore->GetAssetLoadersCore();
    pECS->AddComponent(entity, pAssetLoaders->GetModelLoader()->LoadModel("./assets/Models/Cube.dae"));

    // Attach sound to the cube
    AudioCore* pAudioCore = pEngineCore->GetAudioCore();
    constexpr const float soundVolume = 1.0f;
    pAudioCore->PlayLoopingSound(entity, soundPath, soundVolume);

    return entity;
}
