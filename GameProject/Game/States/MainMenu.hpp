#pragma once

#define NOMINMAX
#include <Engine/ECS/Entity.hpp>
#include <Engine/GameState/State.hpp>

class DeviceDX11;
class InputHandler;

class MainMenu : public State
{
public:
    MainMenu(StateManager* pStateManager, ECSCore* pECS, DeviceDX11* pDevice, InputHandler* pInputHandler);
    ~MainMenu();

    void resume();
    void pause();

    void update(float dt);

    DeviceDX11* getDevice() { return m_pDevice; };
    InputHandler* getInputHandler() { return m_pInputHandler; }

private:
    Entity uiEntity;

    InputHandler* m_pInputHandler;
    DeviceDX11* m_pDevice;
};
