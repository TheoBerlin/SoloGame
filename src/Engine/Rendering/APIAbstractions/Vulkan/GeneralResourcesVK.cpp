#include "GeneralResourcesVK.hpp"

VkPipelineStageFlags convertPipelineStageFlags(PIPELINE_STAGE pipelineStageFlags)
{
    return (int)pipelineStageFlags == 0 ? 0 :
        HAS_FLAG(pipelineStageFlags, PIPELINE_STAGE::TOP_OF_PIPE) * VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT |
        HAS_FLAG(pipelineStageFlags, PIPELINE_STAGE::DRAW_INDIRECT) * VK_PIPELINE_STAGE_DRAW_INDIRECT_BIT |
        HAS_FLAG(pipelineStageFlags, PIPELINE_STAGE::VERTEX_INPUT) * VK_PIPELINE_STAGE_VERTEX_INPUT_BIT |
        HAS_FLAG(pipelineStageFlags, PIPELINE_STAGE::VERTEX_SHADER) * VK_PIPELINE_STAGE_VERTEX_SHADER_BIT |
        HAS_FLAG(pipelineStageFlags, PIPELINE_STAGE::TESSELLATION_CONTROL_SHADER) * VK_PIPELINE_STAGE_TESSELLATION_CONTROL_SHADER_BIT |
        HAS_FLAG(pipelineStageFlags, PIPELINE_STAGE::TESSELLATION_EVALUATION_SHADER) * VK_PIPELINE_STAGE_TESSELLATION_EVALUATION_SHADER_BIT |
        HAS_FLAG(pipelineStageFlags, PIPELINE_STAGE::GEOMETRY_SHADER) * VK_PIPELINE_STAGE_GEOMETRY_SHADER_BIT |
        HAS_FLAG(pipelineStageFlags, PIPELINE_STAGE::FRAGMENT_SHADER) * VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT |
        HAS_FLAG(pipelineStageFlags, PIPELINE_STAGE::EARLY_FRAGMENT_TESTS) * VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT |
        HAS_FLAG(pipelineStageFlags, PIPELINE_STAGE::LATE_FRAGMENT_TESTS) * VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT |
        HAS_FLAG(pipelineStageFlags, PIPELINE_STAGE::COLOR_ATTACHMENT_OUTPUT) * VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT |
        HAS_FLAG(pipelineStageFlags, PIPELINE_STAGE::COMPUTE_SHADER) * VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT |
        HAS_FLAG(pipelineStageFlags, PIPELINE_STAGE::TRANSFER) * VK_PIPELINE_STAGE_TRANSFER_BIT |
        HAS_FLAG(pipelineStageFlags, PIPELINE_STAGE::BOTTOM_OF_PIPE) * VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT |
        HAS_FLAG(pipelineStageFlags, PIPELINE_STAGE::HOST) * VK_PIPELINE_STAGE_HOST_BIT |
        HAS_FLAG(pipelineStageFlags, PIPELINE_STAGE::ALL_GRAPHICS) * VK_PIPELINE_STAGE_ALL_GRAPHICS_BIT |
        HAS_FLAG(pipelineStageFlags, PIPELINE_STAGE::ALL_COMMANDS) * VK_PIPELINE_STAGE_ALL_COMMANDS_BIT |
        HAS_FLAG(pipelineStageFlags, PIPELINE_STAGE::TRANSFORM_FEEDBACK) * VK_PIPELINE_STAGE_TRANSFORM_FEEDBACK_BIT_EXT |
        HAS_FLAG(pipelineStageFlags, PIPELINE_STAGE::CONDITIONAL_RENDERING) * VK_PIPELINE_STAGE_CONDITIONAL_RENDERING_BIT_EXT |
        HAS_FLAG(pipelineStageFlags, PIPELINE_STAGE::RAY_TRACING_SHADER) * VK_PIPELINE_STAGE_RAY_TRACING_SHADER_BIT_KHR |
        HAS_FLAG(pipelineStageFlags, PIPELINE_STAGE::ACCELERATION_STRUCTURE_BUILD) * VK_PIPELINE_STAGE_ACCELERATION_STRUCTURE_BUILD_BIT_NV |
        HAS_FLAG(pipelineStageFlags, PIPELINE_STAGE::SHADING_RATE_IMAGE_NV) * VK_PIPELINE_STAGE_SHADING_RATE_IMAGE_BIT_NV |
        HAS_FLAG(pipelineStageFlags, PIPELINE_STAGE::TASK_SHADER_NV) * VK_PIPELINE_STAGE_TASK_SHADER_BIT_NV |
        HAS_FLAG(pipelineStageFlags, PIPELINE_STAGE::MESH_SHADER_NV) * VK_PIPELINE_STAGE_MESH_SHADER_BIT_NV |
        HAS_FLAG(pipelineStageFlags, PIPELINE_STAGE::FRAGMENT_DENSITY_PROCESS) * VK_PIPELINE_STAGE_FRAGMENT_DENSITY_PROCESS_BIT_EXT |
        HAS_FLAG(pipelineStageFlags, PIPELINE_STAGE::COMMAND_PREPROCESS_NV) * VK_PIPELINE_STAGE_COMMAND_PREPROCESS_BIT_NV |
        HAS_FLAG(pipelineStageFlags, PIPELINE_STAGE::RAY_TRACING_SHADER_NV) * VK_PIPELINE_STAGE_RAY_TRACING_SHADER_BIT_NV |
        HAS_FLAG(pipelineStageFlags, PIPELINE_STAGE::ACCELERATION_STRUCTURE_BUILD_NV) * VK_PIPELINE_STAGE_ACCELERATION_STRUCTURE_BUILD_BIT_NV;
}

VkFormat convertFormatToVK(RESOURCE_FORMAT format)
{
    switch (format) {
        case RESOURCE_FORMAT::R32G32B32A32_FLOAT:
            return VK_FORMAT_R32G32B32A32_SFLOAT;
        case RESOURCE_FORMAT::R32G32B32_FLOAT:
            return VK_FORMAT_R32G32B32_SFLOAT;
        case RESOURCE_FORMAT::R32G32_FLOAT:
            return VK_FORMAT_R32G32_SFLOAT;
        case RESOURCE_FORMAT::B8G8R8A8_UNORM:
            return VK_FORMAT_B8G8R8A8_UNORM;
        case RESOURCE_FORMAT::B8G8R8A8_SRGB:
            return VK_FORMAT_B8G8R8A8_SRGB;
        case RESOURCE_FORMAT::R8G8B8A8_UNORM:
            return VK_FORMAT_R8G8B8A8_UNORM;
        case RESOURCE_FORMAT::R8G8B8A8_SRGB:
            return VK_FORMAT_R8G8B8A8_SRGB;
        case RESOURCE_FORMAT::D32_FLOAT:
            return VK_FORMAT_D32_SFLOAT;
        default:
            LOG_ERROR("Unknown resource format");
            return VK_FORMAT_UNDEFINED;
    }
}

RESOURCE_FORMAT convertFormatFromVK(VkFormat format)
{
    switch (format) {
        case VK_FORMAT_R32G32B32A32_SFLOAT:
            return RESOURCE_FORMAT::R32G32B32A32_FLOAT;
        case VK_FORMAT_R32G32B32_SFLOAT:
            return RESOURCE_FORMAT::R32G32B32_FLOAT;
        case VK_FORMAT_R32G32_SFLOAT:
            return RESOURCE_FORMAT::R32G32_FLOAT;
        case VK_FORMAT_B8G8R8A8_UNORM:
            return RESOURCE_FORMAT::B8G8R8A8_UNORM;
        case VK_FORMAT_B8G8R8A8_SRGB:
            return RESOURCE_FORMAT::B8G8R8A8_SRGB;
        case VK_FORMAT_R8G8B8A8_UNORM:
            return RESOURCE_FORMAT::R8G8B8A8_UNORM;
        case VK_FORMAT_R8G8B8A8_SRGB:
            return RESOURCE_FORMAT::R8G8B8A8_SRGB;
        case VK_FORMAT_D32_SFLOAT:
            return RESOURCE_FORMAT::D32_FLOAT;
        default:
            LOG_ERROR("Unknown resource format");
            return RESOURCE_FORMAT::R8G8B8A8_UNORM;
    }
}

VkCompareOp convertCompareOp(COMPARISON_FUNC comparisonFunc)
{
    switch (comparisonFunc) {
        case COMPARISON_FUNC::NEVER:
            return VK_COMPARE_OP_NEVER;
        case COMPARISON_FUNC::LESS:
            return VK_COMPARE_OP_LESS;
        case COMPARISON_FUNC::LESS_OR_EQUAL:
            return VK_COMPARE_OP_LESS_OR_EQUAL;
        case COMPARISON_FUNC::EQUAL:
            return VK_COMPARE_OP_EQUAL;
        case COMPARISON_FUNC::EQUAL_OR_GREATER:
            return VK_COMPARE_OP_GREATER_OR_EQUAL;
        case COMPARISON_FUNC::GREATER:
            return VK_COMPARE_OP_GREATER;
        case COMPARISON_FUNC::ALWAYS:
            return VK_COMPARE_OP_ALWAYS;
        default:
            LOG_WARNINGF("Erroneous comparison func: %d", (uint32_t)comparisonFunc);
            return VK_COMPARE_OP_LESS;
    }
}

VkImageLayout convertImageLayoutFlag(TEXTURE_LAYOUT layout)
{
    switch (layout) {
        case TEXTURE_LAYOUT::UNDEFINED:
            return VK_IMAGE_LAYOUT_UNDEFINED;
        case TEXTURE_LAYOUT::SHADER_READ_ONLY:
            return VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
        case TEXTURE_LAYOUT::RENDER_TARGET:
            return VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
        case TEXTURE_LAYOUT::DEPTH_ATTACHMENT:
            return VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_OPTIMAL;
        case TEXTURE_LAYOUT::DEPTH_STENCIL_ATTACHMENT:
            return VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
        case TEXTURE_LAYOUT::PRESENT:
            return VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
        default:
            LOG_ERRORF("Unknown image layout flag: %d", (uint32_t)layout);
            return VK_IMAGE_LAYOUT_UNDEFINED;
	}
}

VkAccessFlags convertAccessFlags(RESOURCE_ACCESS accessFlags)
{
    return
        HAS_FLAG(accessFlags, RESOURCE_ACCESS::INDIRECT_COMMAND_READ) * VK_ACCESS_INDIRECT_COMMAND_READ_BIT                     |
        HAS_FLAG(accessFlags, RESOURCE_ACCESS::INDEX_READ) * VK_ACCESS_INDEX_READ_BIT                                           |
        HAS_FLAG(accessFlags, RESOURCE_ACCESS::VERTEX_ATTRIBUTE_READ) * VK_ACCESS_VERTEX_ATTRIBUTE_READ_BIT                     |
        HAS_FLAG(accessFlags, RESOURCE_ACCESS::UNIFORM_READ) * VK_ACCESS_UNIFORM_READ_BIT                                       |
        HAS_FLAG(accessFlags, RESOURCE_ACCESS::INPUT_ATTACHMENT_READ) * VK_ACCESS_INPUT_ATTACHMENT_READ_BIT                     |
        HAS_FLAG(accessFlags, RESOURCE_ACCESS::SHADER_READ) * VK_ACCESS_SHADER_READ_BIT                                         |
        HAS_FLAG(accessFlags, RESOURCE_ACCESS::SHADER_WRITE) * VK_ACCESS_SHADER_WRITE_BIT                                       |
        HAS_FLAG(accessFlags, RESOURCE_ACCESS::COLOR_ATTACHMENT_READ) * VK_ACCESS_COLOR_ATTACHMENT_READ_BIT                     |
        HAS_FLAG(accessFlags, RESOURCE_ACCESS::COLOR_ATTACHMENT_WRITE) * VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT                   |
        HAS_FLAG(accessFlags, RESOURCE_ACCESS::DEPTH_STENCIL_ATTACHMENT_READ) * VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_READ_BIT     |
        HAS_FLAG(accessFlags, RESOURCE_ACCESS::DEPTH_STENCIL_ATTACHMENT_WRITE) * VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT   |
        HAS_FLAG(accessFlags, RESOURCE_ACCESS::TRANSFER_READ) * VK_ACCESS_TRANSFER_READ_BIT                                     |
        HAS_FLAG(accessFlags, RESOURCE_ACCESS::TRANSFER_WRITE) * VK_ACCESS_TRANSFER_WRITE_BIT                                   |
        HAS_FLAG(accessFlags, RESOURCE_ACCESS::HOST_READ) * VK_ACCESS_HOST_READ_BIT                                             |
        HAS_FLAG(accessFlags, RESOURCE_ACCESS::HOST_WRITE) * VK_ACCESS_HOST_WRITE_BIT                                           |
        HAS_FLAG(accessFlags, RESOURCE_ACCESS::MEMORY_READ) * VK_ACCESS_MEMORY_READ_BIT                                         |
        HAS_FLAG(accessFlags, RESOURCE_ACCESS::MEMORY_WRITE) * VK_ACCESS_MEMORY_WRITE_BIT;
}
