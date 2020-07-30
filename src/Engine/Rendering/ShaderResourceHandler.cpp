#include "ShaderResourceHandler.hpp"

#include <Engine/Rendering/APIAbstractions/DX11/DeviceDX11.hpp>
#include <Engine/Rendering/APIAbstractions/DX11/BufferDX11.hpp>
#include <Engine/Rendering/AssetContainers/Model.hpp>
#include <Engine/Utils/DirectXUtils.hpp>
#include <Engine/Utils/ECSUtils.hpp>
#include <Engine/Utils/Logger.hpp>

ShaderResourceHandler::ShaderResourceHandler(ECSCore* pECS, Device* pDevice)
    :ComponentHandler(pECS, TID(ShaderResourceHandler)),
    m_pDevice(pDevice),
    m_pAniSampler(nullptr),
    m_pQuadVertices(nullptr)
{
    ComponentHandlerRegistration handlerReg = {};
    handlerReg.pComponentHandler = this;

    registerHandler(handlerReg);
}

ShaderResourceHandler::~ShaderResourceHandler()
{
    delete m_pQuadVertices;
    delete m_pAniSampler;
}

bool ShaderResourceHandler::initHandler()
{
    SamplerInfo samplerInfo = {};
    samplerInfo.FilterMin           = FILTER::NEAREST;
    samplerInfo.FilterMag           = FILTER::NEAREST;
    samplerInfo.FilterMip           = FILTER::NEAREST;
    samplerInfo.AnisotropyEnabled   = true;
    samplerInfo.MaxAnisotropy       = 1.0f;
    samplerInfo.AddressModeU        = ADDRESS_MODE::MIRROR_REPEAT;
    samplerInfo.AddressModeV        = samplerInfo.AddressModeU;
    samplerInfo.AddressModeW        = samplerInfo.AddressModeU;
    samplerInfo.MipLODBias          = 0.0f;
    samplerInfo.CompareEnabled      = false;
    samplerInfo.ComparisonFunc      = COMPARISON_FUNC::ALWAYS;
    samplerInfo.MinLOD              = 0.0f;
    samplerInfo.MaxLOD              = 0.0f;

    m_pAniSampler = m_pDevice->createSampler(samplerInfo);
    if (!m_pAniSampler) {
        LOG_ERROR("Failed to create anisotropic sampler");
        delete m_pAniSampler;
        return false;
    }

    // Create quad. DirectX's NDC has coordinates in [-1, 1], but here [0, 1]
    // is used as it eases resizing and positioning in the UI vertex shader.
    Vertex2D pQuadVertices[4] = {
        // Position, TXCoord
        {{0.0f, 0.0f}, {0.0f, 1.0f}},
        {{0.0f, 1.0f}, {0.0f, 0.0f}},
        {{1.0f, 0.0f}, {1.0f, 1.0f}},
        {{1.0f, 1.0f}, {1.0f, 0.0f}}
    };

    IBuffer* pQuadBuffer = m_pDevice->createVertexBuffer(pQuadVertices, sizeof(Vertex2D), 4);
    m_pQuadVertices = reinterpret_cast<BufferDX11*>(pQuadBuffer);
    return m_pQuadVertices;
}

IBuffer* ShaderResourceHandler::getQuarterScreenQuad()
{
    return m_pQuadVertices;
}
