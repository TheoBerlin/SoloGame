#pragma once

#include <Engine/Rendering/APIAbstractions/InputLayout.hpp>

#include <vulkan/vulkan.h>

bool convertInputLayoutInfo(VkPipelineVertexInputStateCreateInfo& inputLayoutInfoVK, const InputLayoutInfo& inputLayoutInfo);
