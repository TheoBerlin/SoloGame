#pragma once

#include <Engine/Rendering/APIAbstractions/GeneralResources.hpp>

#include <stdint.h>

enum class STENCIL_OP {
    KEEP,
    ZERO,
    REPLACE,
    INCREMENT_AND_CLAMP,
    DECREMENT_AND_CLAMP,
    INVERT,
    INCREMENT_AND_WRAP,
    DECREMENT_AND_WRAP
};

struct StencilOpInfo {
    STENCIL_OP StencilFailOp;
    STENCIL_OP StencilPassOp;
    STENCIL_OP DepthFailOp;
    COMPARISON_FUNC CompareFunc;
};

struct DepthStencilInfo {
    bool DepthTestEnabled;
    bool DepthWriteEnabled;
    COMPARISON_FUNC DepthComparisonFunc;
    bool StencilTestEnabled;
    StencilOpInfo FrontFace;
    StencilOpInfo BackFace;
    uint32_t Reference;
};
