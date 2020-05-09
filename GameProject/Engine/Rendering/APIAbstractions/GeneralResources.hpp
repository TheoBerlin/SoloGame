#pragma once

#include <Engine/Utils/Logger.hpp>

enum class RESOURCE_FORMAT {
    R32G32B32A32_FLOAT,
    R32G32B32_FLOAT,
    R32G32_FLOAT,
    R8G8B8A8_UNORM,
    D32_FLOAT
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

// Returns size of a format in bytes
size_t getFormatSize(RESOURCE_FORMAT format);
