#include "UIRenderer.hpp"

#include <Engine/ECS/ECSInterface.hpp>
#include <Engine/Rendering/AssetContainers/Model.hpp>
#include <Engine/Rendering/ShaderResourceHandler.hpp>
#include <Engine/Rendering/ShaderHandler.hpp>
#include <Engine/UI/Panel.hpp>
#include <Engine/Utils/DirectXUtils.hpp>
#include <Engine/Utils/Logger.hpp>

UIRenderer::UIRenderer(ECSInterface* ecs, ID3D11DeviceContext* context, ID3D11Device* device, ID3D11RenderTargetView* rtv, ID3D11DepthStencilView* dsv)
    :System(ecs),
    context(context),
    renderTarget(rtv),
    depthStencilView(dsv)
{
    SystemRegistration sysReg = {
    {
        {{{R, tid_UIPanel}}, &panels},
    },
    this};

    this->subscribeToComponents(&sysReg);

    std::type_index tid_shaderResourceHandler = std::type_index(typeid(ShaderResourceHandler));
    std::type_index tid_shaderHandler = std::type_index(typeid(ShaderHandler));
    std::type_index tid_UIHandler = std::type_index(typeid(UIHandler));
    ShaderResourceHandler* shaderResourceHandler = static_cast<ShaderResourceHandler*>(ecs->systemSubscriber.getComponentHandler(tid_shaderResourceHandler));
    this->shaderHandler = static_cast<ShaderHandler*>(ecs->systemSubscriber.getComponentHandler(tid_shaderHandler));
    this->UIhandler = static_cast<UIHandler*>(ecs->systemSubscriber.getComponentHandler(tid_UIHandler));

    this->UIProgram = shaderHandler->getProgram(UI);

    this->quad = shaderResourceHandler->getQuarterScreenQuad();
    this->aniSampler = shaderResourceHandler->getAniSampler();

    // Create per-panel constant buffer
    D3D11_BUFFER_DESC bufferDesc;
    ZeroMemory(&bufferDesc, sizeof(D3D11_BUFFER_DESC));
    bufferDesc.ByteWidth = sizeof(UIPanel) - sizeof(ID3D11ShaderResourceView*);
    bufferDesc.Usage = D3D11_USAGE_DYNAMIC;
    bufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
    bufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
    bufferDesc.MiscFlags = 0;
    bufferDesc.StructureByteStride = 0;

    HRESULT hr = device->CreateBuffer(&bufferDesc, nullptr, perPanelBuffer.GetAddressOf());
    if (FAILED(hr))
        Logger::LOG_ERROR("Failed to create per-object cbuffer: %s", hresultToString(hr).c_str());
}

UIRenderer::~UIRenderer()
{}

void UIRenderer::update(float dt)
{
    if (panels.size() == 0) {
        return;
    }

    context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP);
    context->IASetInputLayout(UIProgram->inputLayout);
    UINT offsets = 0;
    context->IASetVertexBuffers(0, 1, quad.GetAddressOf(), &UIProgram->vertexSize, &offsets);

    context->VSSetShader(UIProgram->vertexShader, nullptr, 0);
    context->HSSetShader(UIProgram->hullShader, nullptr, 0);
    context->DSSetShader(UIProgram->domainShader, nullptr, 0);
    context->GSSetShader(UIProgram->geometryShader, nullptr, 0);
    context->PSSetShader(UIProgram->pixelShader, nullptr, 0);

    context->PSSetSamplers(0, 1, aniSampler);
    context->OMSetRenderTargets(1, &renderTarget, depthStencilView);

    D3D11_MAPPED_SUBRESOURCE mappedResources;
    ZeroMemory(&mappedResources, sizeof(D3D11_MAPPED_SUBRESOURCE));

    for (const Entity& entity : panels.getVec()) {
        UIPanel& panel = UIhandler->panels.indexID(entity);
        // Set per-object buffer
        context->Map(perPanelBuffer.Get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResources);
        memcpy(mappedResources.pData, &panel, sizeof(UIPanel) - sizeof(ID3D11ShaderResourceView*));
        context->Unmap(perPanelBuffer.Get(), 0);

        context->VSSetConstantBuffers(0, 1, perPanelBuffer.GetAddressOf());
        context->PSSetConstantBuffers(0, 1, perPanelBuffer.GetAddressOf());

        context->PSSetShaderResources(0, 1, &panel.texture);

        context->Draw(4, 0);
    }
}
