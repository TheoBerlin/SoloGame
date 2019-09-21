#pragma once

#include <Engine/GameState/State.hpp>
#include <DirectXTK/Keyboard.h>
#include <d3d11.h>

class MainMenu : public State
{
public:
    MainMenu(StateManager* statemanager, ECSInterface* ecs, ID3D11Device* device);
    ~MainMenu();

    void resume();
    void pause();

    void update(float dt);

    ID3D11Device* getDevice();

private:
    DirectX::Keyboard::State* keyboardState;
    ID3D11Device* device;
};
