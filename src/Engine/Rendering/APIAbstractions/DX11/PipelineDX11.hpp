#pragma once

#include <Engine/Rendering/APIAbstractions/DX11/BlendStateDX11.hpp>
#include <Engine/Rendering/APIAbstractions/DX11/DepthStencilStateDX11.hpp>
#include <Engine/Rendering/APIAbstractions/DX11/InputLayoutDX11.hpp>
#include <Engine/Rendering/APIAbstractions/DX11/RasterizerStateDX11.hpp>
#include <Engine/Rendering/APIAbstractions/Pipeline.hpp>
#include <Engine/Rendering/ShaderHandler.hpp>

#define NOMINMAX
#include <d3d11.h>
#include <memory>

struct PipelineInfoDX11 {
    D3D11_PRIMITIVE_TOPOLOGY PrimitiveTopology;
    std::vector<std::shared_ptr<Shader>> Shaders;
    std::vector<Viewport> Viewports;
    std::vector<D3D11_RECT> ScissorRectangles;
    ID3D11RasterizerState* pRasterizerState;
    DepthStencilStateDX11 DepthStencilState;
    BlendStateDX11 BlendState;
};

class DeviceDX11;
class Shader;
class ShaderHandler;

class PipelineDX11 : public IPipeline
{
public:
    static PipelineDX11* create(const PipelineInfo& pipelineInfo, DeviceDX11* pDevice);

public:
    PipelineDX11(const PipelineInfoDX11& pipelineInfo);
    ~PipelineDX11();

    void bind(ID3D11DeviceContext* pContext);

    UINT getVertexSize() const { return m_pInputLayout->VertexSize; }

private:
    PipelineInfoDX11 m_PipelineInfo;

    // Same shaders as the ones in the vector above, explicit d3d11 pointers for faster binding
    ID3D11VertexShader* m_pVertexShader;
    ID3D11HullShader* m_pHullShader;
    ID3D11DomainShader* m_pDomainShader;
    ID3D11GeometryShader* m_pGeometryShader;
    ID3D11PixelShader* m_pFragmentShader;

    InputLayoutDX11* m_pInputLayout;
};
