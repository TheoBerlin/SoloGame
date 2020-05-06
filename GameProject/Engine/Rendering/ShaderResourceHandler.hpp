#pragma once

#define NOMINMAX

#include <Engine/ECS/ComponentHandler.hpp>
#include <d3d11.h>
#include <wrl/client.h>

class BufferDX11;
class Device;
struct Vertex;

class ShaderResourceHandler : public ComponentHandler
{
public:
    ShaderResourceHandler(ECSCore* pECS, Device* pDevice);
    ~ShaderResourceHandler();

    virtual bool initHandler() override;

    ID3D11SamplerState *const* getAniSampler() const;

    BufferDX11* getQuarterScreenQuad();

private:
    Device* m_pDevice;

    /* Samplers */
    Microsoft::WRL::ComPtr<ID3D11SamplerState> aniSampler;

    // Quarter-screen quad
    BufferDX11* m_pQuadVertices;
};
