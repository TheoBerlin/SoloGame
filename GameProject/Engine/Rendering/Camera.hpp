#pragma once

#include <Engine/ECS/System.hpp>
#include <Engine/Utils/IDVector.hpp>
#include <DirectXMath.h>
#include <DirectXTK/Keyboard.h>
#include <DirectXTK/Mouse.h>

const float maxPitch = DirectX::XM_PIDIV2 - 0.01f;
const float g_CameraSpeed = 1.5f;

class TransformHandler;
class VPHandler;

class CameraSystem : public System
{
public:
    CameraSystem(ECSCore* pECS);
    ~CameraSystem();

    virtual bool initSystem() override;

    // Keeps cameras' view matrices up-to date with their transforms
    void update(float dt);

private:
    IDVector m_Cameras;

    TransformHandler* m_pTransformHandler;
    VPHandler* m_pVPHandler;
    DirectX::Keyboard::State* m_pKeyboardState;
    DirectX::Mouse::State* m_pMouseState;
};
