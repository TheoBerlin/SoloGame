#include "DirectXUtils.hpp"

DirectX::XMVECTOR catmullRomDerivative(DirectX::XMVECTOR P0, DirectX::XMVECTOR P1, DirectX::XMVECTOR P2, DirectX::XMVECTOR P3, float T)
{
    float TT = T * T;

    float q0 = -3.0f * TT + 4.0f * T - 1.0f;
    float q1 = 9.0f * TT - 10.0f * T;
    float q2 = -9.0f * TT + 8.0f * T + 1.0f;
    float q3 = 3.0f * TT - 2.0f * T;

    DirectX::XMVECTOR result = DirectX::XMVectorScale(P0, q0);
    result = DirectX::XMVectorAdd(DirectX::XMVectorScale(P1, q1), result);
    result = DirectX::XMVectorAdd(DirectX::XMVectorScale(P2, q2), result);
    result = DirectX::XMVectorAdd(DirectX::XMVectorScale(P3, q3), result);

    return DirectX::XMVectorScale(result, 0.5f);
}
