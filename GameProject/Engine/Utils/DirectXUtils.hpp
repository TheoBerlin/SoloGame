#pragma once

#include <DirectXMath.h>
#include <system_error>
#include <winerror.h>

inline std::string hresultToString(HRESULT hr)
{
    return std::system_category().message(hr);
}

DirectX::XMVECTOR catmullRomDerivative(DirectX::XMVECTOR P0, DirectX::XMVECTOR P1, DirectX::XMVECTOR P2, DirectX::XMVECTOR P3, float T);
