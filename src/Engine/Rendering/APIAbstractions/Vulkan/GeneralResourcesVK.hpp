#pragma once

#include <Engine/Rendering/APIAbstractions/GeneralResources.hpp>

#include <vulkan/vulkan.h>

VkPipelineStageFlags convertPipelineStageFlags(PIPELINE_STAGE pipelineStageFlags);

VkFormat convertFormatToVK(RESOURCE_FORMAT format);
RESOURCE_FORMAT convertFormatFromVK(VkFormat format);
VkCompareOp convertCompareOp(COMPARISON_FUNC comparisonFunc);
VkImageLayout convertImageLayoutFlag(TEXTURE_LAYOUT layout);
VkAccessFlags convertAccessFlags(RESOURCE_ACCESS accessFlags);
