#pragma once

#include <stdint.h>

enum class SHADER_BINDING : uint32_t {
    PER_FRAME           = 0u,
    SAMPLER_ONE         = 1u,
    PER_OBJECT          = 2u,
    MATERIAL_CONSTANTS  = 3u,
    TEXTURE_ONE         = 4u
};
