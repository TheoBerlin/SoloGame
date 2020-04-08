#include "Renderer.hpp"

#include <Engine/Utils/DirectXUtils.hpp>
#include <Engine/Utils/Logger.hpp>

Renderer::Renderer(ECSCore* pECS)
    :m_pECS(pECS),
    m_pDeferredContext(nullptr)
{}

Renderer::~Renderer()
{
    if (m_pDeferredContext) {
        m_pDeferredContext->Release();
    }
}

bool Renderer::init(ID3D11Device* pDevice)
{
    HRESULT hr = pDevice->CreateDeferredContext(0, &m_pDeferredContext);
    if (FAILED(hr)) {
        LOG_ERROR("Failed to create deferred context: %s", hresultToString(hr).c_str());
        return false;
    }

    return true;
}
