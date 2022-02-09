#include "ShaderResourceHandler.hpp"

#include <Engine/Rendering/APIAbstractions/DX11/DeviceDX11.hpp>
#include <Engine/Rendering/APIAbstractions/DX11/BufferDX11.hpp>
#include <Engine/Rendering/AssetContainers/Model.hpp>
#include <Engine/Utils/DirectXUtils.hpp>
#include <Engine/Utils/ECSUtils.hpp>

ShaderResourceHandler ShaderResourceHandler::s_Instance;

ShaderResourceHandler::ShaderResourceHandler()
    :m_pDevice(nullptr),
    m_pAniSampler(nullptr),
    m_pQuadVertices(nullptr)
{}

bool ShaderResourceHandler::Init(Device* pDevice)
{
    m_pDevice = pDevice;

    const SamplerInfo samplerInfo = {
        .FilterMin           = FILTER::NEAREST,
        .FilterMag           = FILTER::NEAREST,
        .FilterMip           = FILTER::NEAREST,
        .AnisotropyEnabled   = true,
        .MaxAnisotropy       = 1.0f,
        .AddressModeU        = ADDRESS_MODE::MIRROR_REPEAT,
        .AddressModeV        = samplerInfo.AddressModeU,
        .AddressModeW        = samplerInfo.AddressModeU,
        .MipLODBias          = 0.0f,
        .CompareEnabled      = false,
        .ComparisonFunc      = COMPARISON_FUNC::ALWAYS,
        .MinLOD              = 0.0f,
        .MaxLOD              = 0.0f
    };

    m_pAniSampler = m_pDevice->createSampler(samplerInfo);
    if (!m_pAniSampler) {
        LOG_ERROR("Failed to create anisotropic sampler");
        delete m_pAniSampler;
        return false;
    }

    // Create quad. DirectX's NDC has coordinates in [-1, 1], but here [0, 1]
    // is used as it eases resizing and positioning in the UI vertex shader.
    const Vertex2D pQuadVertices[4] = {
        // Position, TXCoord
        {{0.0f, 0.0f}, {0.0f, 1.0f}},
        {{0.0f, 1.0f}, {0.0f, 0.0f}},
        {{1.0f, 0.0f}, {1.0f, 1.0f}},
        {{1.0f, 1.0f}, {1.0f, 0.0f}}
    };

    m_pQuadVertices = m_pDevice->createVertexBuffer(pQuadVertices, sizeof(Vertex2D), 4);

    return m_pQuadVertices;
}

void ShaderResourceHandler::Release()
{
    delete m_pQuadVertices;
    delete m_pAniSampler;
}

IBuffer* ShaderResourceHandler::GetQuarterScreenQuad()
{
    return m_pQuadVertices;
}
