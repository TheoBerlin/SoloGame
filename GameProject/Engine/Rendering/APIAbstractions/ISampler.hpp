#pragma once

#include <Engine/Utils/EnumClass.hpp>

#include <stdint.h>

enum class FILTER {
    NEAREST,
    LINEAR
};

enum class ADDRESS_MODE {
    REPEAT,
    MIRROR_REPEAT,
    MIRROR_CLAMP_TO_EDGE,
    CLAMP_TO_EDGE,
    CLAMP_TO_BORDER
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

struct SamplerInfo {
    FILTER FilterMin;
    FILTER FilterMag;
    FILTER FilterMip;
    bool AnisotropyEnabled;
    float MaxAnisotropy;
    ADDRESS_MODE AddressModeU;
    ADDRESS_MODE AddressModeV;
    ADDRESS_MODE AddressModeW;
    float MipLODBias;
    bool CompareEnabled;
    COMPARISON_FUNC ComparisonFunc;
    float* pBorderColor;
    float MinLOD;
    float MaxLOD;
};

class ISampler
{
public:
    virtual ~ISampler() = 0 {};
};
