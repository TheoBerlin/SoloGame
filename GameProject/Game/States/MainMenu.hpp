#pragma once

#define NOMINMAX
#include <Engine/ECS/Entity.hpp>
#include <Engine/GameState/State.hpp>
#include <DirectXTK/Keyboard.h>
#include <d3d11.h>

class InputHandler;

class MainMenu : public State
{
public:
    MainMenu(StateManager* statemanager, ECSCore* ecs, ID3D11Device* device);
    ~MainMenu();

    void resume();
    void pause();

    void update(float dt);

    ID3D11Device* getDevice();

private:
    Entity uiEntity;

    InputHandler* inputHandler;
    DirectX::Keyboard::State* keyboardState;
    ID3D11Device* device;
};
