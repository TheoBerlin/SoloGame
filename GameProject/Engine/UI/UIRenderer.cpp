#include "UIRenderer.hpp"

#include <Engine/Rendering/APIAbstractions/Device.hpp>
#include <Engine/Rendering/APIAbstractions/DX11/CommandListDX11.hpp>
#include <Engine/Rendering/APIAbstractions/IRasterizerState.hpp>
#include <Engine/Rendering/AssetContainers/Model.hpp>
#include <Engine/Rendering/ShaderResourceHandler.hpp>
#include <Engine/Rendering/ShaderHandler.hpp>
#include <Engine/Rendering/Window.hpp>
#include <Engine/UI/Panel.hpp>
#include <Engine/Utils/DirectXUtils.hpp>
#include <Engine/Utils/ECSUtils.hpp>
#include <Engine/Utils/Logger.hpp>

UIRenderer::UIRenderer(ECSCore* pECS, Device* pDevice, Window* pWindow)
    :Renderer(pECS, pDevice),
    m_pCommandList(nullptr),
    m_pRenderTarget(pDevice->getBackBuffer()),
    m_pDepthStencil(pDevice->getDepthStencil()),
    m_BackbufferWidth(pWindow->getWidth()),
    m_BackbufferHeight(pWindow->getHeight()),
    m_pQuad(nullptr),
    m_pPerPanelBuffer(nullptr)
{
    RendererRegistration rendererReg = {};
    rendererReg.SubscriberRegistration.ComponentSubscriptionRequests = {
        {{{R, tid_UIPanel}}, &m_Panels}
    };
    rendererReg.pRenderer = this;

    registerRenderer(rendererReg);
}

UIRenderer::~UIRenderer()
{
    delete m_pCommandList;
    delete m_pPerPanelBuffer;
    delete m_pRasterizerState;
}

bool UIRenderer::init()
{
    m_pCommandList = m_pDevice->createCommandList();
    if (!m_pCommandList) {
        return false;
    }

    ShaderResourceHandler* pShaderResourceHandler = static_cast<ShaderResourceHandler*>(getComponentHandler(TID(ShaderResourceHandler)));
    m_pShaderHandler = static_cast<ShaderHandler*>(getComponentHandler(TID(ShaderHandler)));
    m_pUIHandler = static_cast<UIHandler*>(getComponentHandler(TID(UIHandler)));
    if (!m_pShaderHandler || !m_pUIHandler) {
        return false;
    }

    m_pUIProgram = m_pShaderHandler->getProgram(UI);

    m_pQuad = pShaderResourceHandler->getQuarterScreenQuad();
    m_ppAniSampler = pShaderResourceHandler->getAniSampler();

    // Create per-panel constant buffer
    BufferInfo bufferInfo = {};
    bufferInfo.ByteSize = sizeof(
        DirectX::XMFLOAT2) * 2 +    // Position and size
        sizeof(DirectX::XMFLOAT4) + // Highlight color
        sizeof(float) +             // Highlight factor
        sizeof(DirectX::XMFLOAT3    // Padding
    );
    bufferInfo.GPUAccess    = BUFFER_DATA_ACCESS::READ;
    bufferInfo.CPUAccess    = BUFFER_DATA_ACCESS::WRITE;
    bufferInfo.Usage        = BUFFER_USAGE::UNIFORM_BUFFER;

    m_pPerPanelBuffer = m_pDevice->createBuffer(bufferInfo);
    if (!m_pPerPanelBuffer) {
        LOG_ERROR("Failed to create per-object unfiform buffer");
        return false;
    }

    RasterizerStateInfo rsInfo = {};
    rsInfo.PolygonMode          = POLYGON_MODE::FILL;
    rsInfo.CullMode             = CULL_MODE::NONE;
    rsInfo.FrontFaceOrientation = FRONT_FACE_ORIENTATION::CLOCKWISE;
    rsInfo.DepthBiasEnable      = false;

    m_pRasterizerState = m_pDevice->createRasterizerState(rsInfo);
    if (!m_pRasterizerState) {
        LOG_ERROR("Failed to create rasterizer state");
        return false;
    }

    // Create viewport
    m_Viewport = {};
    m_Viewport.TopLeftX = 0;
    m_Viewport.TopLeftY = 0;
    m_Viewport.Width = (float)m_BackbufferWidth;
    m_Viewport.Height = (float)m_BackbufferHeight;
    m_Viewport.MinDepth = 0.0f;
    m_Viewport.MaxDepth = 1.0f;

    return true;
}

void UIRenderer::recordCommands()
{
    if (m_Panels.size() == 0) {
        return;
    }

    ID3D11DeviceContext* pContext = reinterpret_cast<CommandListDX11*>(m_pCommandList)->getContext();

    pContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP);
    pContext->IASetInputLayout(m_pUIProgram->inputLayout);

    m_pCommandList->bindVertexBuffer(0, m_pUIProgram->vertexSize, m_pQuad);

    m_pCommandList->bindShaders(m_pUIProgram);

    m_pCommandList->bindRasterizerState(m_pRasterizerState);
    pContext->RSSetViewports(1, &m_Viewport);

    pContext->PSSetSamplers(0, 1, m_ppAniSampler);
    m_pCommandList->bindRenderTarget(m_pRenderTarget, m_pDepthStencil);

    size_t bufferSize = sizeof(
        DirectX::XMFLOAT2) * 2 +    // Position and size
        sizeof(DirectX::XMFLOAT4) + // Highlight color
        sizeof(float);              // Highlight factor

    for (const Entity& entity : m_Panels.getIDs()) {
        UIPanel& panel = m_pUIHandler->panels.indexID(entity);
        if (!panel.texture) {
            continue;
        }

        // Set per-object buffer
        void* pMappedBuffer = nullptr;
        m_pCommandList->map(m_pPerPanelBuffer, &pMappedBuffer);
        memcpy(pMappedBuffer, &panel, bufferSize);
        m_pCommandList->unmap(m_pPerPanelBuffer);

        m_pCommandList->bindBuffer(0, SHADER_TYPE::VERTEX_SHADER | SHADER_TYPE::FRAGMENT_SHADER, m_pPerPanelBuffer);

        m_pCommandList->bindShaderResourceTexture(0, SHADER_TYPE::FRAGMENT_SHADER, panel.texture);

        m_pCommandList->draw(4);
    }
}

void UIRenderer::executeCommands()
{
    m_pCommandList->execute();
}
