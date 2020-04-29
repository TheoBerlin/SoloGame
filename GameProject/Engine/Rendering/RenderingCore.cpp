#include "RenderingCore.hpp"

RenderingCore::RenderingCore(ECSCore* pECS, ID3D11Device* pDevice)
    :m_VPHandler(pECS),
    m_ShaderHandler(pDevice, pECS),
    m_ShaderResourceHandler(pECS, pDevice),
    m_RenderableHandler(pECS),
    m_LightHandler(pECS),
    m_CameraSystem(pECS)
{}

RenderingCore::~RenderingCore()
{}
