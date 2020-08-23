#pragma once

#include <Engine/ECS/Entity.hpp>
#include <Engine/GameState/State.hpp>

class Device;
class InputHandler;

class MainMenu : public State
{
public:
    MainMenu(StateManager* pStateManager, ECSCore* pECS, Device* pDevice, InputHandler* pInputHandler);
    ~MainMenu() = default;

    void init() override final;

    void resume() override final;
    void pause() override final;

    void update(float dt) override final;

    inline Device* getDevice()             { return m_pDevice; };
    inline InputHandler* getInputHandler() { return m_pInputHandler; }

private:
    void createGameSession();

private:
    Entity uiEntity;

    InputHandler* m_pInputHandler;
    Device* m_pDevice;
};
