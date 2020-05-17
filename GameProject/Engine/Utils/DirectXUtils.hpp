#pragma once

#include <Engine/Rendering/APIAbstractions/Texture.hpp>
#include <Engine/Utils/EnumClass.hpp>

#define NOMINMAX
#include <d3d11.h>
#include <DirectXMath.h>
#include <system_error>
#include <winerror.h>

#define SAFERELEASE(pCOMObject) if (pCOMObject) { pCOMObject->Release(); }

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

inline std::string hresultToString(HRESULT hr)
{
    return std::system_category().message(hr);
}

DirectX::XMVECTOR catmullRomDerivative(DirectX::XMVECTOR P0, DirectX::XMVECTOR P1, DirectX::XMVECTOR P2, DirectX::XMVECTOR P3, float T);
