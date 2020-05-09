#pragma once

#include <Engine/ECS/ComponentHandler.hpp>

#define NOMINMAX
#include <d3d11.h>

class IBuffer;
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

    IBuffer* getQuarterScreenQuad();

private:
    Device* m_pDevice;

    ISampler* m_pAniSampler;

    // Quarter-screen quad
    IBuffer* m_pQuadVertices;
};
