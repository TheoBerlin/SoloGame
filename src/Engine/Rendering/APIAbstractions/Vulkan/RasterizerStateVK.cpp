#include "RasterizerStateVK.hpp"

bool convertRasterizerStateInfo(VkPipelineRasterizationStateCreateInfo& rasterizerState, const RasterizerStateInfo& rasterizerStateInfo)
{
    rasterizerState = {};
    rasterizerState.sType                   = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
    rasterizerState.polygonMode             = rasterizerStateInfo.PolygonMode == POLYGON_MODE::FILL ? VK_POLYGON_MODE_FILL : VK_POLYGON_MODE_LINE;
    rasterizerState.cullMode                = convertCullMode(rasterizerStateInfo.CullMode);
    rasterizerState.frontFace               = rasterizerStateInfo.FrontFaceOrientation == FRONT_FACE_ORIENTATION::CLOCKWISE ? VK_FRONT_FACE_CLOCKWISE : VK_FRONT_FACE_COUNTER_CLOCKWISE;
    rasterizerState.depthBiasEnable         = rasterizerStateInfo.DepthBiasEnable ? VK_TRUE : VK_FALSE;
    rasterizerState.depthBiasConstantFactor = rasterizerStateInfo.DepthBiasConstantFactor;
    rasterizerState.depthBiasClamp          = rasterizerStateInfo.DepthBiasClamp;
    rasterizerState.lineWidth               = rasterizerStateInfo.LineWidth;

    return true;
}

VkCullModeFlags convertCullMode(CULL_MODE cullMode)
{
    switch (cullMode) {
        case CULL_MODE::NONE:
            return VK_CULL_MODE_NONE;
        case CULL_MODE::FRONT:
            return VK_CULL_MODE_FRONT_BIT;
        case CULL_MODE::BACK:
            return VK_CULL_MODE_BACK_BIT;
        default:
            LOG_WARNINGF("Eroneous cull mode: %d", (uint32_t)cullMode);
            return VK_CULL_MODE_BACK_BIT;
    }
}
