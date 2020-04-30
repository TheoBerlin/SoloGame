#pragma once

#include <Engine/Rendering/Camera.hpp>
#include <Engine/Rendering/Components/PointLight.hpp>
#include <Engine/Rendering/Components/Renderable.hpp>
#include <Engine/Rendering/Components/VPMatrices.hpp>
#include <Engine/Rendering/ShaderHandler.hpp>
#include <Engine/Rendering/ShaderResourceHandler.hpp>

class Window;

class RenderingCore
{
public:
    RenderingCore(ECSCore* pECS, IDevice* pDevice, Window* pWindow);
    ~RenderingCore();

private:
    // Component Handlers
    VPHandler m_VPHandler;
    ShaderHandler m_ShaderHandler;
    ShaderResourceHandler m_ShaderResourceHandler;
    RenderableHandler m_RenderableHandler;
    LightHandler m_LightHandler;

    // Systems
    CameraSystem m_CameraSystem;
};
