#pragma once

#include <Engine/ECS/System.hpp>
#include <Engine/Utils/IDVector.hpp>
#include <DirectXMath.h>
#include <DirectXTK/Keyboard.h>
#include <DirectXTK/Mouse.h>

const float maxPitch = DirectX::XM_PIDIV2 - 0.01f;

class TransformHandler;
class VPHandler;

class CameraSystem : public System
{
public:
    CameraSystem(ECSInterface* ecs);
    ~CameraSystem();

    void update(float dt);

private:
    IDVector<Entity> cameras;

    TransformHandler* transformHandler;
    VPHandler *vpHandler;
    DirectX::Keyboard::State* keyboardState;
    DirectX::Mouse::State* mouseState;
};
