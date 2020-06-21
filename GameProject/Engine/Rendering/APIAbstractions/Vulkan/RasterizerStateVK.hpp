#pragma once

#include <Engine/Rendering/APIAbstractions/RasterizerState.hpp>

#include <vulkan/vulkan.h>

bool convertRasterizerStateInfo(VkPipelineRasterizationStateCreateInfo& rasterizerState, const RasterizerStateInfo& rasterizerStateInfo);

VkCullModeFlags convertCullMode(CULL_MODE cullMode);
