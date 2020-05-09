#pragma once

#include <Engine/Rendering/APIAbstractions/GeneralResources.hpp>
#include <Engine/Utils/EnumClass.hpp>

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
