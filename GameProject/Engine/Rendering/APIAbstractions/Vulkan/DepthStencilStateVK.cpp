#include "DepthStencilStateVK.hpp"

#include <Engine/Rendering/APIAbstractions/Vulkan/GeneralResourcesVK.hpp>

bool createDepthStencilState(VkPipelineDepthStencilStateCreateInfo& depthStencilState, const DepthStencilInfo& depthStencilInfo)
{
    depthStencilState = {};
    depthStencilState.sType                 = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
    depthStencilState.depthTestEnable       = depthStencilInfo.DepthTestEnabled ? VK_TRUE : VK_FALSE;
    depthStencilState.depthWriteEnable      = depthStencilInfo.DepthWriteEnabled ? VK_TRUE : VK_FALSE;
    depthStencilState.depthCompareOp        = convertCompareOp(depthStencilInfo.DepthComparisonFunc);
    depthStencilState.depthBoundsTestEnable = VK_FALSE;
    depthStencilState.stencilTestEnable     = depthStencilInfo.StencilTestEnabled ? VK_TRUE : VK_FALSE;
    if (depthStencilInfo.StencilTestEnabled) {
        depthStencilState.front = convertStencilOpState(depthStencilInfo.FrontFace);
        depthStencilState.back  = convertStencilOpState(depthStencilInfo.BackFace);
    }

    return true;
}

VkStencilOpState convertStencilOpState(const StencilOpInfo& stencilOpInfo)
{
    VkStencilOpState stencilOpState = {};
    stencilOpState.failOp       = convertStencilOp(stencilOpInfo.StencilFailOp);
    stencilOpState.passOp       = convertStencilOp(stencilOpInfo.StencilPassOp);
    stencilOpState.depthFailOp  = convertStencilOp(stencilOpInfo.DepthFailOp);
    stencilOpState.compareOp    = convertCompareOp(stencilOpInfo.CompareFunc);
    stencilOpState.writeMask    = UINT32_MAX;
    stencilOpState.reference    = 1u;

    return stencilOpState;
}

VkStencilOp convertStencilOp(STENCIL_OP stencilOp)
{
    switch (stencilOp) {
        case STENCIL_OP::KEEP:
            return VK_STENCIL_OP_KEEP;
        case STENCIL_OP::ZERO:
            return VK_STENCIL_OP_ZERO;
        case STENCIL_OP::REPLACE:
            return VK_STENCIL_OP_REPLACE;
        case STENCIL_OP::INCREMENT_AND_CLAMP:
            return VK_STENCIL_OP_INCREMENT_AND_CLAMP;
        case STENCIL_OP::DECREMENT_AND_CLAMP:
            return VK_STENCIL_OP_DECREMENT_AND_CLAMP;
        case STENCIL_OP::INVERT:
            return VK_STENCIL_OP_INVERT;
        case STENCIL_OP::INCREMENT_AND_WRAP:
            return VK_STENCIL_OP_INCREMENT_AND_WRAP;
        case STENCIL_OP::DECREMENT_AND_WRAP:
            return VK_STENCIL_OP_DECREMENT_AND_WRAP;
        default:
            LOG_WARNING("Erroneous stencil op: %d", (uint32_t)stencilOp);
            return VK_STENCIL_OP_REPLACE;
    }
}
