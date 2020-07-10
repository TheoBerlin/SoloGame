#pragma once

#include <Engine/Utils/EnumClass.hpp>
#include <Engine/Utils/Logger.hpp>

#include <glm/glm.hpp>

#define MAX_FRAMES_IN_FLIGHT 3u

enum class RESOURCE_FORMAT {
    R32G32B32A32_FLOAT,
    R32G32B32_FLOAT,
    R32G32_FLOAT,
    B8G8R8A8_UNORM,
    B8G8R8A8_SRGB,
    R8G8B8A8_UNORM,
    R8G8B8A8_SRGB,
    D32_FLOAT
};

enum class FORMAT_PRIMITIVE_TYPE {
    INTEGER,
    UNSIGNED_INTEGER,
    FLOAT
};

enum class COMPARISON_FUNC {
    NEVER,
    LESS,
    LESS_OR_EQUAL,
    EQUAL,
    EQUAL_OR_GREATER,
    GREATER,
    ALWAYS
};

enum class PIPELINE_STAGE : uint32_t {
    TOP_OF_PIPE                     = 1,
    DRAW_INDIRECT                   = TOP_OF_PIPE << 1,
    VERTEX_INPUT                    = DRAW_INDIRECT << 1,
    VERTEX_SHADER                   = VERTEX_INPUT << 1,
    TESSELLATION_CONTROL_SHADER     = VERTEX_SHADER << 1,
    TESSELLATION_EVALUATION_SHADER  = TESSELLATION_CONTROL_SHADER << 1,
    GEOMETRY_SHADER                 = TESSELLATION_EVALUATION_SHADER << 1,
    FRAGMENT_SHADER                 = GEOMETRY_SHADER << 1,
    EARLY_FRAGMENT_TESTS            = FRAGMENT_SHADER << 1,
    LATE_FRAGMENT_TESTS             = EARLY_FRAGMENT_TESTS << 1,
    COLOR_ATTACHMENT_OUTPUT         = LATE_FRAGMENT_TESTS << 1,
    COMPUTE_SHADER                  = COLOR_ATTACHMENT_OUTPUT << 1,
    TRANSFER                        = COMPUTE_SHADER << 1,
    BOTTOM_OF_PIPE                  = TRANSFER << 1,
    HOST                            = BOTTOM_OF_PIPE << 1,
    ALL_GRAPHICS                    = HOST << 1,
    ALL_COMMANDS                    = ALL_GRAPHICS << 1,
    TRANSFORM_FEEDBACK              = ALL_COMMANDS << 1,
    CONDITIONAL_RENDERING           = TRANSFORM_FEEDBACK << 1,
    RAY_TRACING_SHADER              = CONDITIONAL_RENDERING << 1,
    ACCELERATION_STRUCTURE_BUILD    = RAY_TRACING_SHADER << 1,
    SHADING_RATE_IMAGE_NV           = ACCELERATION_STRUCTURE_BUILD << 1,
    TASK_SHADER_NV                  = SHADING_RATE_IMAGE_NV << 1,
    MESH_SHADER_NV                  = TASK_SHADER_NV << 1,
    FRAGMENT_DENSITY_PROCESS        = MESH_SHADER_NV << 1,
    COMMAND_PREPROCESS_NV           = FRAGMENT_DENSITY_PROCESS << 1,
    RAY_TRACING_SHADER_NV           = COMMAND_PREPROCESS_NV << 1,
    ACCELERATION_STRUCTURE_BUILD_NV = RAY_TRACING_SHADER_NV << 1
};

DEFINE_BITMASK_OPERATIONS(PIPELINE_STAGE)

enum class DEPENDENCY_FLAG : uint32_t {
    BY_REGION       = 1,
    DEVICE_GROUP    = BY_REGION << 1,
    VIEW_LOCAL      = DEVICE_GROUP << 1
};

DEFINE_BITMASK_OPERATIONS(DEPENDENCY_FLAG)

enum class RESOURCE_ACCESS : uint32_t {
    INDIRECT_COMMAND_READ               = 1,
    INDEX_READ                          = INDIRECT_COMMAND_READ << 1,
    VERTEX_ATTRIBUTE_READ               = INDEX_READ << 1,
    UNIFORM_READ                        = VERTEX_ATTRIBUTE_READ << 1,
    INPUT_ATTACHMENT_READ               = UNIFORM_READ << 1,
    SHADER_READ                         = INPUT_ATTACHMENT_READ << 1,
    SHADER_WRITE                        = SHADER_READ << 1,
    COLOR_ATTACHMENT_READ               = SHADER_WRITE << 1,
    COLOR_ATTACHMENT_WRITE              = COLOR_ATTACHMENT_READ << 1,
    DEPTH_STENCIL_ATTACHMENT_READ       = COLOR_ATTACHMENT_WRITE << 1,
    DEPTH_STENCIL_ATTACHMENT_WRITE      = DEPTH_STENCIL_ATTACHMENT_READ << 1,
    TRANSFER_READ                       = DEPTH_STENCIL_ATTACHMENT_WRITE << 1,
    TRANSFER_WRITE                      = TRANSFER_READ << 1,
    HOST_READ                           = TRANSFER_WRITE << 1,
    HOST_WRITE                          = HOST_READ << 1,
    MEMORY_READ                         = HOST_WRITE << 1,
    MEMORY_WRITE                        = MEMORY_READ << 1
};

DEFINE_BITMASK_OPERATIONS(RESOURCE_ACCESS)

enum class SHARING_MODE {
    EXCLUSIVE   = 0,
    CONCURRENT  = 1
};

struct Rectangle2D {
    glm::ivec2 Offset;
    glm::uvec2 Extent;
};

// Returns size of a format in bytes
size_t getFormatSize(RESOURCE_FORMAT format);
FORMAT_PRIMITIVE_TYPE getFormatPrimitiveType(RESOURCE_FORMAT format);
