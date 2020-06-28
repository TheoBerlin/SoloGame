#pragma once

#include <Engine/Utils/EnumClass.hpp>

#include <stdint.h>
#include <vector>

enum class BLEND_FACTOR {
    ZERO,
    ONE,
    SRC_COLOR,
    ONE_MINUS_SRC_COLOR,
    DST_COLOR,
    ONE_MINUS_DST_COLOR,
    SRC_ALPHA,
    ONE_MINUS_SRC_ALPHA,
    DST_ALPHA,
    ONE_MINUS_DST_ALPHA,
    CONSTANT_COLOR,
    ONE_MINUS_CONSTANT_COLOR,
    CONSTANT_ALPHA,
    ONE_MINUS_CONSTANT_ALPHA,
    SRC_ALPHA_SATURATE,
    SRC1_COLOR,
    ONE_MINUS_SRC1_COLOR,
    SRC1_ALPHA,
    ONE_MINUS_SRC1_ALPHA
};

enum class BLEND_OP {
    ADD,
    SUBTRACT,
    REVERSE_SUBTRACT,
    MIN,
    MAX
};

enum class COLOR_WRITE_MASK : uint8_t {
    RED         = 1,
    GREEN       = RED << 1,
    BLUE        = GREEN << 1,
    ALPHA       = BLUE << 1,
    ENABLE_ALL  = RED + GREEN + BLUE + ALPHA
};

DEFINE_BITMASK_OPERATIONS(COLOR_WRITE_MASK)

struct BlendRenderTargetInfo {
    bool BlendEnabled;
    BLEND_FACTOR SrcColorBlendFactor;
    BLEND_FACTOR DstColorBlendFactor;
    BLEND_OP ColorBlendOp;
    BLEND_FACTOR SrcAlphaBlendFactor;
    BLEND_FACTOR DstAlphaBlendFactor;
    BLEND_OP AlphaBlendOp;
    COLOR_WRITE_MASK ColorWriteMask;
};

struct BlendStateInfo {
    std::vector<BlendRenderTargetInfo> RenderTargetBlendInfos;
    bool IndependentBlendEnabled;
    float pBlendConstants[4];
};
