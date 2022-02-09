#pragma once

#include <Engine/GameState/State.hpp>
#include <Game/Level/Tube.hpp>
#include <Game/LightSpinner.hpp>
#include <Game/Racer/Components/Track.hpp>
#include <Game/Racer/Systems/RacerController.hpp>

class InputHandler;
class MainMenuState;
class ModelLoader;
class SoundHandler;

class GameSession : public State
{
public:
    GameSession(MainMenuState* pMainMenu);
    ~GameSession() = default;

    void Init() override final;

    void Resume() override final;
    void Pause() override final;

    void Update(float dt) override final;

private:
    void CreatePointLights();
    void CreateTube(const std::vector<DirectX::XMFLOAT3>& sectionPoints);
    void CreatePlayer();

private:
    TubeHandler m_TubeHandler;

    // Systems
    LightSpinner m_LightSpinner;
    RacerController m_RacerController;
};
