#include "BlendStateVK.hpp"

bool convertBlendStateInfo(VkPipelineColorBlendStateCreateInfo& blendStateInfoVK, std::vector<VkPipelineColorBlendAttachmentState>& attachmentStates, const BlendStateInfo& blendStateInfo)
{
    attachmentStates.reserve(blendStateInfo.RenderTargetBlendInfos.size());

    for (const BlendRenderTargetInfo& renderTargetBlendInfo : blendStateInfo.RenderTargetBlendInfos) {
        VkPipelineColorBlendAttachmentState blendAttachmentState = {};
        blendAttachmentState.blendEnable            = renderTargetBlendInfo.BlendEnabled;
        blendAttachmentState.srcColorBlendFactor    = convertBlendFactor(renderTargetBlendInfo.SrcColorBlendFactor);
        blendAttachmentState.dstColorBlendFactor    = convertBlendFactor(renderTargetBlendInfo.DstColorBlendFactor);
        blendAttachmentState.colorBlendOp           = convertBlendOp(renderTargetBlendInfo.ColorBlendOp);
        blendAttachmentState.srcAlphaBlendFactor    = convertBlendFactor(renderTargetBlendInfo.SrcAlphaBlendFactor);
        blendAttachmentState.dstAlphaBlendFactor    = convertBlendFactor(renderTargetBlendInfo.DstAlphaBlendFactor);
        blendAttachmentState.alphaBlendOp           = convertBlendOp(renderTargetBlendInfo.AlphaBlendOp);
        blendAttachmentState.colorWriteMask         = convertColorWriteMask(renderTargetBlendInfo.ColorWriteMask);

        attachmentStates.push_back(blendAttachmentState);
    }

    blendStateInfoVK = {};
    blendStateInfoVK.sType              = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
    blendStateInfoVK.logicOpEnable      = VK_FALSE;
    blendStateInfoVK.attachmentCount    = (uint32_t)attachmentStates.size();
    blendStateInfoVK.pAttachments       = attachmentStates.data();
    std::memcpy(blendStateInfoVK.blendConstants, blendStateInfo.pBlendConstants, sizeof(float) * 4u);

    return true;
}

VkBlendFactor convertBlendFactor(BLEND_FACTOR blendFactor)
{
    switch (blendFactor) {
        case BLEND_FACTOR::ZERO:
            return VK_BLEND_FACTOR_ZERO;
        case BLEND_FACTOR::ONE:
            return VK_BLEND_FACTOR_ONE;
        case BLEND_FACTOR::SRC_COLOR:
            return VK_BLEND_FACTOR_SRC_COLOR;
        case BLEND_FACTOR::ONE_MINUS_SRC_COLOR:
            return VK_BLEND_FACTOR_ONE_MINUS_SRC_COLOR;
        case BLEND_FACTOR::DST_COLOR:
            return VK_BLEND_FACTOR_DST_COLOR;
        case BLEND_FACTOR::ONE_MINUS_DST_COLOR:
            return VK_BLEND_FACTOR_ONE_MINUS_DST_COLOR;
        case BLEND_FACTOR::SRC_ALPHA:
            return VK_BLEND_FACTOR_SRC_ALPHA;
        case BLEND_FACTOR::ONE_MINUS_SRC_ALPHA:
            return VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
        case BLEND_FACTOR::DST_ALPHA:
            return VK_BLEND_FACTOR_DST_ALPHA;
        case BLEND_FACTOR::ONE_MINUS_DST_ALPHA:
            return VK_BLEND_FACTOR_ONE_MINUS_DST_ALPHA;
        case BLEND_FACTOR::CONSTANT_COLOR:
            return VK_BLEND_FACTOR_CONSTANT_COLOR;
        case BLEND_FACTOR::CONSTANT_ALPHA:
            return VK_BLEND_FACTOR_CONSTANT_ALPHA;
        case BLEND_FACTOR::ONE_MINUS_CONSTANT_COLOR:
            return VK_BLEND_FACTOR_ONE_MINUS_CONSTANT_COLOR;
        case BLEND_FACTOR::ONE_MINUS_CONSTANT_ALPHA:
            return VK_BLEND_FACTOR_ONE_MINUS_CONSTANT_ALPHA;
        case BLEND_FACTOR::SRC_ALPHA_SATURATE:
            return VK_BLEND_FACTOR_SRC_ALPHA_SATURATE;
        case BLEND_FACTOR::SRC1_COLOR:
            return VK_BLEND_FACTOR_SRC1_COLOR;
        case BLEND_FACTOR::ONE_MINUS_SRC1_COLOR:
            return VK_BLEND_FACTOR_ONE_MINUS_SRC1_COLOR;
        case BLEND_FACTOR::SRC1_ALPHA:
            return VK_BLEND_FACTOR_SRC1_ALPHA;
        case BLEND_FACTOR::ONE_MINUS_SRC1_ALPHA:
            return VK_BLEND_FACTOR_ONE_MINUS_SRC1_ALPHA;
        default:
            LOG_WARNINGF("Erroneous blend factor: %d", (int)blendFactor);
            return VK_BLEND_FACTOR_ONE;
    }
}

VkBlendOp convertBlendOp(BLEND_OP blendOp)
{
    switch (blendOp) {
        case BLEND_OP::ADD:
            return VK_BLEND_OP_ADD;
        case BLEND_OP::SUBTRACT:
            return VK_BLEND_OP_SUBTRACT;
        case BLEND_OP::REVERSE_SUBTRACT:
            return VK_BLEND_OP_REVERSE_SUBTRACT;
        case BLEND_OP::MIN:
            return VK_BLEND_OP_MIN;
        case BLEND_OP::MAX:
            return VK_BLEND_OP_MAX;
        default:
            LOG_WARNINGF("Erroneous blend op: %d", (int)blendOp);
            return VK_BLEND_OP_ADD;
    }
}

VkColorComponentFlags convertColorWriteMask(COLOR_WRITE_MASK colorWriteMask)
{
    return
        HAS_FLAG(colorWriteMask, COLOR_WRITE_MASK::RED)     * VK_COLOR_COMPONENT_R_BIT |
        HAS_FLAG(colorWriteMask, COLOR_WRITE_MASK::GREEN)   * VK_COLOR_COMPONENT_G_BIT |
        HAS_FLAG(colorWriteMask, COLOR_WRITE_MASK::BLUE)    * VK_COLOR_COMPONENT_B_BIT |
        HAS_FLAG(colorWriteMask, COLOR_WRITE_MASK::ALPHA)   * VK_COLOR_COMPONENT_A_BIT;
}
