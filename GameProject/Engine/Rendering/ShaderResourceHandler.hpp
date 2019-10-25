#pragma once

#define NOMINMAX

#include <Engine/ECS/ComponentHandler.hpp>
#include <d3d11.h>
#include <wrl/client.h>

struct Vertex;

class ShaderResourceHandler : public ComponentHandler
{
public:
    ShaderResourceHandler(SystemSubscriber* sysSubscriber, ID3D11Device* device);
    ~ShaderResourceHandler();

    void createVertexBuffer(const void* vertices, size_t vertexSize, size_t vertexCount, ID3D11Buffer** targetBuffer);
    void createIndexBuffer(unsigned* indices, size_t indexCount, ID3D11Buffer** targetBuffer);

    ID3D11SamplerState *const* getAniSampler() const;

private:
    ID3D11Device* device;

    /* Samplers */
    Microsoft::WRL::ComPtr<ID3D11SamplerState> aniSampler;
};
