#pragma once

#include <Engine/Rendering/Window.hpp>

class CameraSystem;
class Device;

struct EngineConfig;

class RenderingCore
{
public:
    RenderingCore();
    ~RenderingCore();

    bool Init(const EngineConfig& engineConfig);

    Window* GetWindow()             { return &m_Window; }
    Device* GetDevice()             { return m_pDevice; }
    CameraSystem* GetCameraSystem() { return m_pCameraSystem; }

private:
    Window m_Window;
    Device* m_pDevice;

    // Systems
    CameraSystem* m_pCameraSystem;
};
