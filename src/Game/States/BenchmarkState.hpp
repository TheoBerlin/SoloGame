#pragma once

#include <Engine/GameState/State.hpp>
#include <Game/Level/Tube.hpp>
#include <Game/LightSpinner.hpp>
#include <Game/Racer/Components/Track.hpp>
#include <Game/Racer/Systems/RacerController.hpp>

class AssetLoadersCore;
class AudioCore;
class Device;
class InputHandler;
class ModelLoader;
class RenderingCore;
class RuntimeStats;

class BenchmarkState : public State
{
public:
    BenchmarkState(StateManager* pStateManager, const RuntimeStats* pRuntimeStats);
    ~BenchmarkState() = default;

    void Init() override final;

    void Resume() override final;
    void Pause() override final;

    void Update(float dt) override final;

private:
    void CreatePointLights();
    void CreateTube(const std::vector<DirectX::XMFLOAT3>& sectionPoints);
    void CreatePlayer();

    void PrintBenchmarkResults() const;

private:
    const RuntimeStats* m_pRuntimeStats;

    Entity m_PlayerEntity;

    TubeHandler m_TubeHandler;

    // Systems
    LightSpinner m_LightSpinner;
    RacerController m_RacerController;
};
