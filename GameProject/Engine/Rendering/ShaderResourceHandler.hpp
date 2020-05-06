#pragma once

#include <Engine/ECS/ComponentHandler.hpp>

#define NOMINMAX
#include <d3d11.h>

class BufferDX11;
class Device;
class ISampler;
struct Vertex;

class ShaderResourceHandler : public ComponentHandler
{
public:
    ShaderResourceHandler(ECSCore* pECS, Device* pDevice);
    ~ShaderResourceHandler();

    virtual bool initHandler() override;

    ISampler* getAniSampler() { return m_pAniSampler; }

    BufferDX11* getQuarterScreenQuad();

private:
    Device* m_pDevice;

    ISampler* m_pAniSampler;

    // Quarter-screen quad
    BufferDX11* m_pQuadVertices;
};
