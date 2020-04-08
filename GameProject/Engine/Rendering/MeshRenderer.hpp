#pragma once

#define NOMINMAX
#include <Engine/ECS/System.hpp>
#include <Engine/Rendering/Components/PointLight.hpp>
#include <d3d11.h>

#define MAX_POINTLIGHTS 7

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

class MeshRenderer : public System
{
public:
    MeshRenderer(ECSCore* pECS, ID3D11Device* pDevice, ID3D11DeviceContext* pContext, ID3D11RenderTargetView* pRTV, ID3D11DepthStencilView* pDSV);
    ~MeshRenderer();

    virtual bool init() override;

    void update(float dt);

private:
    IDVector<Entity> m_Renderables;
    IDVector<Entity> m_Camera;
    IDVector<Entity> m_PointLights;

    RenderableHandler* m_pRenderableHandler;
    TransformHandler* m_pTransformHandler;
    VPHandler* m_pVPHandler;
    LightHandler* m_pLightHandler;

    ID3D11Device* m_pDevice;
    ID3D11DeviceContext* m_pContext;

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
};
