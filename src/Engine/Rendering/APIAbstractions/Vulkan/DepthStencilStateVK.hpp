#pragma once

#include <Engine/Rendering/APIAbstractions/DepthStencilState.hpp>

#include <vulkan/vulkan.h>

bool convertDepthStencilStateInfo(VkPipelineDepthStencilStateCreateInfo& depthStencilState, const DepthStencilInfo& depthStencilInfo);

VkStencilOpState convertStencilOpState(const StencilOpInfo& stencilOpInfo);
VkStencilOp convertStencilOp(STENCIL_OP stencilOp);
