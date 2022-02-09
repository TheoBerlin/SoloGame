#include "RenderingCore.hpp"

#include <Engine/Rendering/Camera.hpp>
#include <Engine/Rendering/Components/PointLight.hpp>
#include <Engine/Rendering/Components/VPMatrices.hpp>
#include <Engine/Rendering/ShaderResourceHandler.hpp>

RenderingCore::RenderingCore()
    :   m_Window(720u, 16.0f / 9.0f)
    ,   m_pDevice(nullptr)
    ,   m_pCameraSystem(nullptr)
{}

RenderingCore::~RenderingCore()
{
    delete m_pCameraSystem;
    ShaderResourceHandler::GetInstance()->Release();
    delete m_pDevice;
}

bool RenderingCore::Init(const EngineConfig& engineConfig)
{
    m_pCameraSystem = DBG_NEW CameraSystem(m_Window.GetInputHandler());

    LOG_INFOF("Using %s", engineConfig.RenderingAPI == RENDERING_API::VULKAN ? "Vulkan" : "DirectX 11");

    if (!m_Window.init()) {
        return false;
    }

    const SwapchainInfo swapchainInfo = {
        .FrameRateLimit    = 60u,
        .Multisamples      = 1u,
        .PresentationMode  = engineConfig.PresentationMode,
        .Windowed          = true
    };

    DescriptorCounts descriptorPoolSize;
    descriptorPoolSize.setAll(100u);

    m_pDevice = Device::create(engineConfig.RenderingAPI, swapchainInfo, &m_Window);
    if (!m_pDevice || !m_pDevice->init(descriptorPoolSize)) {
        return false;
    }

    return ShaderResourceHandler::GetInstance()->Init(m_pDevice);
}
