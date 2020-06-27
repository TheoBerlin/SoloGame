#pragma once

#include <Engine/Rendering/APIAbstractions/InputLayout.hpp>

#include <vulkan/vulkan.h>

bool convertInputLayoutInfo(VkPipelineVertexInputStateCreateInfo& inputLayoutInfoVK, std::vector<VkVertexInputAttributeDescription>& attributeDescs, const InputLayoutInfo& inputLayoutInfo);
