#pragma once

#define NOMINMAX
#include <Engine/ECS/Entity.hpp>
#include <Engine/GameState/State.hpp>

class Device;
class InputHandler;

class MainMenu : public State
{
public:
    MainMenu(StateManager* pStateManager, ECSCore* pECS, Device* pDevice, InputHandler* pInputHandler);
    ~MainMenu();

    void resume() override final;
    void pause() override final;

    void update(float dt) override final;

    Device* getDevice() { return m_pDevice; };
    InputHandler* getInputHandler() { return m_pInputHandler; }

private:
    Entity uiEntity;

    InputHandler* m_pInputHandler;
    Device* m_pDevice;
};
