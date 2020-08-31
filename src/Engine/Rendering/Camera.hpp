#pragma once

#include <Engine/ECS/System.hpp>
#include <Engine/Utils/IDVector.hpp>
#include <DirectXMath.h>

const float maxPitch = DirectX::XM_PIDIV2 - 0.01f;
const float g_CameraSpeed = 1.5f;

class InputHandler;
class TransformHandler;
class VelocityHandler;
class VPHandler;

class CameraSystem : public System
{
public:
    CameraSystem(ECSCore* pECS, InputHandler* pInputHandler);
    ~CameraSystem() = default;

    virtual bool initSystem() override;

    // Keeps cameras' view matrices up-to date with their transforms
    void update(float dt);

private:
    IDVector m_Cameras;

    TransformHandler* m_pTransformHandler;
    VelocityHandler* m_pVelocityHandler;
    VPHandler* m_pVPHandler;
    InputHandler* m_pInputHandler;
};
