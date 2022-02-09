#pragma once

#include <Engine/ECS/System.hpp>
#include <Engine/Utils/IDVector.hpp>
#include <DirectXMath.h>

constexpr const float g_MaxPitch = DirectX::XM_PIDIV2 - 0.01f;
constexpr const float g_CameraSpeed = 1.5f;

class InputHandler;

struct CameraTagComponent {
    DECL_COMPONENT(CameraTagComponent);
};

class CameraSystem : public System
{
public:
    CameraSystem(InputHandler* pInputHandler);
    ~CameraSystem() = default;

    // Keeps cameras' view matrices up-to date with their transforms
    void Update(float dt);

private:
    IDVector m_Cameras;

    InputHandler* m_pInputHandler;
};
