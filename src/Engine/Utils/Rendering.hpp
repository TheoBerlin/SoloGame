#pragma once

#define ACTION_PER_CONTAINED_SHADER(shaderStages, VSAction, HSAction, DSAction, GSAction, FSAction) \
    if (HAS_FLAG(shaderStages, SHADER_TYPE::VERTEX_SHADER)) {                                       \
        VSAction;                                                                                   \
    }                                                                                               \
                                                                                                    \
    if (HAS_FLAG(shaderStages, SHADER_TYPE::HULL_SHADER)) {                                         \
        HSAction;                                                                                   \
    }                                                                                               \
                                                                                                    \
    if (HAS_FLAG(shaderStages, SHADER_TYPE::DOMAIN_SHADER)) {                                       \
        DSAction;                                                                                   \
    }                                                                                               \
                                                                                                    \
    if (HAS_FLAG(shaderStages, SHADER_TYPE::GEOMETRY_SHADER)) {                                     \
        GSAction;                                                                                   \
    }                                                                                               \
                                                                                                    \
    if (HAS_FLAG(shaderStages, SHADER_TYPE::FRAGMENT_SHADER)) {                                     \
        FSAction;                                                                                   \
    }
