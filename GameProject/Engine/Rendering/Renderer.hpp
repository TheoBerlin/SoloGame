#pragma once

#define NOMINMAX
#include <Engine/ECS/System.hpp>
#include <Engine/Rendering/Components/PointLight.hpp>
#include <d3d11.h>

#define MAX_POINTLIGHTS 7

class RenderableHandler;
class TransformHandler;
class VPHandler;

class Renderer : public System
{
public:
    Renderer(ECSCore* pECS, ID3D11Device* device, ID3D11DeviceContext* context, ID3D11RenderTargetView* rtv, ID3D11DepthStencilView* dsv);
    ~Renderer();

    void update(float dt);

private:
    RenderableHandler* renderableHandler;
    TransformHandler* transformHandler;
    VPHandler* vpHandler;
    LightHandler* lightHandler;

    IDVector<Entity> renderables;
    IDVector<Entity> camera;
    IDVector<Entity> pointLights;

    ID3D11DeviceContext* context;

    /* Mesh input layout */
    ID3D11InputLayout* meshInputLayout;

    /* Cbuffers */
    // WVP and W matrices
    ID3D11Buffer* perObjectMatrices;
    ID3D11Buffer* materialBuffer;
    // Contains pointlights, camera position and number of lights
    ID3D11Buffer* pointLightBuffer;

    /* Samplers */
    ID3D11SamplerState *const* aniSampler;

    /* Render targets */
    ID3D11RenderTargetView* renderTarget;
    ID3D11DepthStencilView* depthStencilView;

    ID3D11RasterizerState* rsState;
};

struct PerObjectMatrices {
    DirectX::XMFLOAT4X4 WVP, world;
};

struct PerFrameBuffer {
    PointLight pointLights[MAX_POINTLIGHTS];
    DirectX::XMFLOAT3 cameraPosition;
    uint32_t numLights;
    DirectX::XMFLOAT4 padding;
};
