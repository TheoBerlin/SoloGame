#pragma once

#include <Engine/GameState/State.hpp>
#include <Game/Level/Tube.hpp>
#include <Game/LightSpinner.hpp>
#include <Game/Racer/Components/Track.hpp>
#include <Game/Racer/Systems/RacerController.hpp>

class Device;
class InputHandler;
class ModelLoader;
class RuntimeStats;
class SoundHandler;
class Window;

class Benchmark : public State
{
public:
    Benchmark(StateManager* pStateManager, ECSCore* pECS, Device* pDevice, InputHandler* pInputHandler, const RuntimeStats* pRuntimeStats, Window* pWindow);
    ~Benchmark() = default;

    void init() override final;

    void resume() override final;
    void pause() override final;

    void update(float dt) override final;

private:
    void startMusic(SoundHandler* pSoundHandler);
    void createCube(const DirectX::XMFLOAT3& position, const std::string& soundPath, SoundHandler* pSoundHandler, TransformHandler* pTransformHandler, ModelLoader* pModelLoader);
    void createPointLights(SoundHandler* pSoundHandler, TransformHandler* pTransformHandler, EntityPublisher* pEntityPublisher);
    void createTube(const std::vector<DirectX::XMFLOAT3>& sectionPoints, TransformHandler* pTransformHandler, ModelLoader* pModelLoader);
    void createPlayer(TransformHandler* pTransformHandler, EntityPublisher* pEntityPublisher);

    void printBenchmarkResults() const;

private:
    InputHandler* m_pInputHandler;
    Device* m_pDevice;

    const RuntimeStats* m_pRuntimeStats;
    Window* m_pWindow;

    Entity m_PlayerEntity;

    // Component handlers
    TrackHandler m_TrackPositionHandler;
    TubeHandler m_TubeHandler;

    // Systems
    LightSpinner m_LightSpinner;
    RacerController m_RacerController;
};
