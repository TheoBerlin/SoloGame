#pragma once

#include <Engine/ECS/System.hpp>
#include <Engine/Utils/IDVector.hpp>
#include <DirectXTK/Keyboard.h>

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
};
