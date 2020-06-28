#include "RenderingCore.hpp"

#include <Engine/Rendering/Window.hpp>

RenderingCore::RenderingCore(ECSCore* pECS, Device* pDevice, Window* pWindow)
    :m_VPHandler(pECS),
    m_ShaderResourceHandler(pECS, pDevice),
    m_LightHandler(pECS),
    m_CameraSystem(pECS, pWindow->getInputHandler())
{}

RenderingCore::~RenderingCore()
{}
