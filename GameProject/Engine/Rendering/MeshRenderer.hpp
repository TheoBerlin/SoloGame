#pragma once

#define NOMINMAX
#include <Engine/ECS/Renderer.hpp>
#include <Engine/Rendering/Components/PointLight.hpp>
#include <d3d11.h>

#define MAX_POINTLIGHTS 7u

class Display;
class RenderableHandler;
class TransformHandler;
class VPHandler;

struct PerObjectMatrices {
    DirectX::XMFLOAT4X4 WVP, world;
};

struct PerFrameBuffer {
    PointLight pointLights[MAX_POINTLIGHTS];
    DirectX::XMFLOAT3 cameraPosition;
    uint32_t numLights;
    DirectX::XMFLOAT4 padding;
};

class MeshRenderer : public Renderer
{
public:
    MeshRenderer(ECSCore* pECS, Display* pDisplay);
    ~MeshRenderer();

    virtual bool init() override;
    virtual void recordCommands() override;
    virtual bool executeCommands() override;

private:
    IDVector m_Renderables;
    IDVector m_Camera;
    IDVector m_PointLights;

    ID3D11DeviceContext* m_pCommandBuffer;

    RenderableHandler* m_pRenderableHandler;
    TransformHandler* m_pTransformHandler;
    VPHandler* m_pVPHandler;
    LightHandler* m_pLightHandler;

    /* Mesh input layout */
    ID3D11InputLayout* m_pMeshInputLayout;

    /* Cbuffers */
    // WVP and W matrices
    ID3D11Buffer* m_pPerObjectMatrices;
    ID3D11Buffer* m_pMaterialBuffer;
    // Contains pointlights, camera position and number of lights
    ID3D11Buffer* m_pPointLightBuffer;

    /* Samplers */
    ID3D11SamplerState *const* m_ppAniSampler;

    /* Render targets */
    ID3D11RenderTargetView* m_pRenderTarget;
    ID3D11DepthStencilView* m_pDepthStencilView;

    ID3D11RasterizerState* m_RsState;

    D3D11_VIEWPORT m_Viewport;
    unsigned int m_BackbufferWidth, m_BackbufferHeight;
};
