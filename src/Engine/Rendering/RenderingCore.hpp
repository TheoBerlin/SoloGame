#pragma once

#include <Engine/Rendering/Camera.hpp>
#include <Engine/Rendering/Components/PointLight.hpp>
#include <Engine/Rendering/Components/VPMatrices.hpp>
#include <Engine/Rendering/ShaderResourceHandler.hpp>

class Window;

class RenderingCore
{
public:
    RenderingCore(ECSCore* pECS, Device* pDevice, Window* pWindow);
    ~RenderingCore();

private:
    // Component Handlers
    VPHandler m_VPHandler;
    ShaderResourceHandler m_ShaderResourceHandler;
    LightHandler m_LightHandler;

    // Systems
    CameraSystem m_CameraSystem;
};
