#pragma once

#include <Engine/Rendering/APIAbstractions/GeneralResources.hpp>

#include <vulkan/vulkan.h>

VkPipelineStageFlags convertPipelineStageFlags(PIPELINE_STAGE pipelineStageFlags);
