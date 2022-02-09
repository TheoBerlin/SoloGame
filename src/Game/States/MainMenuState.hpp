#pragma once

#include <Engine/ECS/Entity.hpp>
#include <Engine/GameState/State.hpp>

class MainMenuState : public State
{
public:
    MainMenuState(StateManager* pStateManager);
    ~MainMenuState() = default;

    void Init() override final;

    void Resume() override final;
    void Pause() override final;

    void Update(float dt) override final;

private:
    void CreatePlayButton();

    void createGameSession();

private:
    Entity m_PlayButtonEntity;
};
