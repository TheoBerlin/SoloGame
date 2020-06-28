#pragma once

#include <Engine/Rendering/APIAbstractions/BlendState.hpp>

#include <vulkan/vulkan.h>

bool convertBlendStateInfo(VkPipelineColorBlendStateCreateInfo& blendStateInfoVK, std::vector<VkPipelineColorBlendAttachmentState>& attachmentStates, const BlendStateInfo& blendStateInfo);

VkBlendFactor convertBlendFactor(BLEND_FACTOR blendFactor);
VkBlendOp convertBlendOp(BLEND_OP blendOp);
VkColorComponentFlags convertColorWriteMask(COLOR_WRITE_MASK colorWriteMask);
