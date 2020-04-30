#pragma once

#define NOMINMAX
#include <Engine/ECS/Entity.hpp>
#include <Engine/GameState/State.hpp>

#include <d3d11.h>

class InputHandler;

class MainMenu : public State
{
public:
    MainMenu(StateManager* pStateManager, ECSCore* pECS, ID3D11Device* pDevice, InputHandler* pInputHandler);
    ~MainMenu();

    void resume();
    void pause();

    void update(float dt);

    ID3D11Device* getDevice();
    InputHandler* getInputHandler() { return m_pInputHandler; }

private:
    Entity uiEntity;

    InputHandler* m_pInputHandler;
    ID3D11Device* device;
};
