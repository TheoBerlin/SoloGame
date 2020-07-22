#pragma once

#include <Engine/Rendering/APIAbstractions/InputLayout.hpp>

#include <vulkan/vulkan.h>

struct InputLayoutInfoVK {
    VkPipelineVertexInputStateCreateInfo VertexInputState;
    VkVertexInputBindingDescription InputBindingDescription;
    std::vector<VkVertexInputAttributeDescription> AttributeDescriptions;
};

bool convertInputLayoutInfo(InputLayoutInfoVK& inputLayoutInfoVK, const InputLayoutInfo& inputLayoutInfo);
