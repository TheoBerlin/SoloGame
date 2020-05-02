#pragma once

#define NOMINMAX

#include <Engine/ECS/ComponentHandler.hpp>
#include <d3d11.h>
#include <wrl/client.h>

class BufferDX11;
class IDevice;
struct Vertex;

class ShaderResourceHandler : public ComponentHandler
{
public:
    ShaderResourceHandler(ECSCore* pECS, IDevice* pDevice);
    ~ShaderResourceHandler();

    virtual bool initHandler() override;

    ID3D11SamplerState *const* getAniSampler() const;

    BufferDX11* getQuarterScreenQuad();

private:
    IDevice* m_pDevice;

    /* Samplers */
    Microsoft::WRL::ComPtr<ID3D11SamplerState> aniSampler;

    // Quarter-screen quad
    BufferDX11* m_pQuadVertices;
};
