#include "PipelineDX11.hpp"

#include <Engine/Rendering/APIAbstractions/DX11/DeviceDX11.hpp>
#include <Engine/Utils/Debug.hpp>
#include <Engine/Utils/DirectXUtils.hpp>

PipelineDX11* PipelineDX11::create(const PipelineInfo& pipelineInfo, DeviceDX11* pDevice)
{
    // Convert abstract PipelineInfo struct to PipelineInfoDX11
    PipelineInfoDX11 pipelineInfoDX = {};
    pipelineInfoDX.PrimitiveTopology = convertPrimitiveTopology(pipelineInfo.PrimitiveTopology);

    ShaderHandler* pShaderHandler = pDevice->getShaderHandler();
    pipelineInfoDX.Shaders.reserve(pipelineInfo.ShaderInfos.size());
    for (const ShaderInfo& shaderInfo : pipelineInfo.ShaderInfos) {
        if (shaderInfo.ShaderType == SHADER_TYPE::VERTEX_SHADER) {
            pipelineInfoDX.VertexStage = pShaderHandler->loadVertexStage(shaderInfo.ShaderName);
        } else {
            pipelineInfoDX.Shaders.push_back(pShaderHandler->loadShader(shaderInfo.ShaderName, shaderInfo.ShaderType));
        }
    }

    pipelineInfoDX.Shaders.shrink_to_fit();

    pipelineInfoDX.Viewports            = pipelineInfo.Viewports;
    pipelineInfoDX.pRasterizerState     = pDevice->createRasterizerState(pipelineInfo.RasterizerStateInfo);
    if (!createDepthStencilState(pipelineInfoDX.DepthStencilState, pipelineInfo.DepthStencilStateInfo, pDevice->getDevice())) {
        return nullptr;
    }

    if (!createBlendState(pipelineInfoDX.BlendState, pipelineInfo.BlendStateInfo, pDevice->getDevice())) {
        return nullptr;
    }

    return DBG_NEW PipelineDX11(pipelineInfoDX);
}

PipelineDX11::PipelineDX11(const PipelineInfoDX11& pipelineInfo)
    :m_PipelineInfo(pipelineInfo),
    m_pVertexShader(nullptr),
    m_pHullShader(nullptr),
    m_pDomainShader(nullptr),
    m_pGeometryShader(nullptr),
    m_pFragmentShader(nullptr)
{
    if (pipelineInfo.VertexStage) {
        m_pVertexShader = reinterpret_cast<ShaderDX11*>(pipelineInfo.VertexStage->getVertexShader())->getVertexShader();
    }

    for (std::shared_ptr<Shader>& shader : m_PipelineInfo.Shaders) {
        ShaderDX11* pShader = reinterpret_cast<ShaderDX11*>(shader.get());

        switch (pShader->getShaderType()) {
            case SHADER_TYPE::HULL_SHADER:
                m_pHullShader = pShader->getHullShader();
                break;
            case SHADER_TYPE::DOMAIN_SHADER:
                m_pDomainShader = pShader->getDomainShader();
                break;
            case SHADER_TYPE::GEOMETRY_SHADER:
                m_pGeometryShader = pShader->getGeometryShader();
                break;
            case SHADER_TYPE::FRAGMENT_SHADER:
                m_pFragmentShader = pShader->getFragmentShader();
                break;
        }
    }
}

PipelineDX11::~PipelineDX11()
{
    delete m_PipelineInfo.pRasterizerState;
    SAFERELEASE(m_PipelineInfo.DepthStencilState.pDepthStencilState)
    SAFERELEASE(m_PipelineInfo.BlendState.pBlendState)
}

void PipelineDX11::bind(ID3D11DeviceContext* pContext)
{
    pContext->VSSetShader(m_pVertexShader, nullptr, 0);
    pContext->HSSetShader(m_pHullShader, nullptr, 0);
    pContext->DSSetShader(m_pDomainShader, nullptr, 0);
    pContext->GSSetShader(m_pGeometryShader, nullptr, 0);
    pContext->PSSetShader(m_pFragmentShader, nullptr, 0);

    InputLayoutDX11* pInputLayout = reinterpret_cast<InputLayoutDX11*>(m_PipelineInfo.VertexStage->getInputLayout());
    pContext->IASetInputLayout(pInputLayout->getInputLayout());
    pContext->IASetPrimitiveTopology(m_PipelineInfo.PrimitiveTopology);

    pContext->RSSetState(m_PipelineInfo.pRasterizerState->getRasterizerState());
    pContext->RSSetViewports((UINT)m_PipelineInfo.Viewports.size(), (const D3D11_VIEWPORT*)m_PipelineInfo.Viewports.data());

    pContext->OMSetDepthStencilState(m_PipelineInfo.DepthStencilState.pDepthStencilState, m_PipelineInfo.DepthStencilState.StencilReference);
    pContext->OMSetBlendState(m_PipelineInfo.BlendState.pBlendState, m_PipelineInfo.BlendState.pBlendConstants, D3D11_COLOR_WRITE_ENABLE_ALL);
}
