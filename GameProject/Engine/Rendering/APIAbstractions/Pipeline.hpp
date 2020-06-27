#pragma once

#include <Engine/Rendering/APIAbstractions/BlendState.hpp>
#include <Engine/Rendering/APIAbstractions/DepthStencilState.hpp>
#include <Engine/Rendering/APIAbstractions/InputLayout.hpp>
#include <Engine/Rendering/APIAbstractions/RasterizerState.hpp>
#include <Engine/Rendering/APIAbstractions/Shader.hpp>
#include <Engine/Rendering/APIAbstractions/Viewport.hpp>

#include <string>
#include <vector>

struct ShaderInfo {
    std::string ShaderName;
    SHADER_TYPE ShaderType;
};

enum class PIPELINE_DYNAMIC_STATE {
    VIEWPORT,
    SCISSOR
};

class IPipelineLayout;
class IRenderPass;

struct PipelineInfo {
    std::vector<ShaderInfo> ShaderInfos;
    PRIMITIVE_TOPOLOGY PrimitiveTopology;
    std::vector<Viewport> Viewports;
    RasterizerStateInfo RasterizerStateInfo;
    DepthStencilInfo DepthStencilStateInfo;
    BlendStateInfo BlendStateInfo;
    std::vector<PIPELINE_DYNAMIC_STATE> DynamicStates;
    IPipelineLayout* pLayout;
    IRenderPass* pRenderPass;
    uint32_t Subpass;
};


class IPipeline
{
public:
    virtual ~IPipeline() = 0 {};
};
