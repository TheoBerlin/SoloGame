#pragma once

#include <Engine/Utils/EnumClass.hpp>

#include <stdint.h>

enum class SHADER_TYPE : uint32_t {
    VERTEX_SHADER   = 1,
    HULL_SHADER     = VERTEX_SHADER << 1,
    DOMAIN_SHADER   = HULL_SHADER   << 1,
    GEOMETRY_SHADER = DOMAIN_SHADER << 1,
    FRAGMENT_SHADER = GEOMETRY_SHADER << 1
};

DEFINE_BITMASK_OPERATIONS(SHADER_TYPE)
