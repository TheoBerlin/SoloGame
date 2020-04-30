#pragma once

#define NOMINMAX

#include <Engine/ECS/ComponentHandler.hpp>
#include <d3d11.h>
#include <wrl/client.h>

class IDevice;
struct Vertex;

class ShaderResourceHandler : public ComponentHandler
{
public:
    ShaderResourceHandler(ECSCore* pECS, IDevice* pDevice);
    ~ShaderResourceHandler();

    virtual bool initHandler() override;

    bool createVertexBuffer(const void* vertices, size_t vertexSize, size_t vertexCount, ID3D11Buffer** targetBuffer);
    bool createIndexBuffer(unsigned* indices, size_t indexCount, ID3D11Buffer** targetBuffer);

    ID3D11SamplerState *const* getAniSampler() const;

    Microsoft::WRL::ComPtr<ID3D11Buffer> getQuarterScreenQuad();

private:
    IDevice* m_pDevice;

    /* Samplers */
    Microsoft::WRL::ComPtr<ID3D11SamplerState> aniSampler;

    // Quarter-screen quad
    Microsoft::WRL::ComPtr<ID3D11Buffer> quad;
};
