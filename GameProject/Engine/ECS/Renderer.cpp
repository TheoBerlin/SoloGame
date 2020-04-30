#include "Renderer.hpp"

#include <Engine/ECS/ECSCore.hpp>
#include <Engine/Rendering/APIAbstractions/DX11/DeviceDX11.hpp>
#include <Engine/Utils/DirectXUtils.hpp>
#include <Engine/Utils/Logger.hpp>

Renderer::Renderer(ECSCore* pECS, IDevice* pDevice)
    :m_pECS(pECS),
    m_pDevice(pDevice)
{
    DeviceDX11* pDeviceDX = reinterpret_cast<DeviceDX11*>(pDevice);
    m_pImmediateContext = pDeviceDX->getContext();
}

Renderer::~Renderer()
{
    m_pECS->getComponentSubscriber()->unsubscribeFromComponents(m_ComponentSubscriptionID);
}

void Renderer::registerRenderer(const RendererRegistration& rendererRegistration)
{
    m_pECS->enqueueRendererRegistration(rendererRegistration);
}

ComponentHandler* Renderer::getComponentHandler(const std::type_index& handlerType)
{
    return m_pECS->getComponentSubscriber()->getComponentHandler(handlerType);
}

bool Renderer::createCommandBuffer(ID3D11DeviceContext** ppCommandBuffer)
{
    DeviceDX11* pDeviceDX = reinterpret_cast<DeviceDX11*>(m_pDevice);
    ID3D11Device* pDevice = pDeviceDX->getDevice();

    HRESULT hr = pDevice->CreateDeferredContext(0, ppCommandBuffer);
    if (FAILED(hr)) {
        LOG_ERROR("Failed to create deferred context: %s", hresultToString(hr).c_str());
        return false;
    }

    return true;
}

bool Renderer::executeCommandBuffer(ID3D11DeviceContext* pCommandBuffer)
{
    ID3D11CommandList* pCommandList = nullptr;
    HRESULT hr = pCommandBuffer->FinishCommandList(FALSE, &pCommandList);
    if (FAILED(hr)) {
        LOG_WARNING("Failed to finish command list: %s", hresultToString(hr).c_str());
        return false;
    }

    m_pImmediateContext->ExecuteCommandList(pCommandList, FALSE);
    pCommandList->Release();
    return true;
}
