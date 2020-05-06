#pragma once

#define NOMINMAX
#include <Engine/ECS/Renderer.hpp>
#include <Engine/Rendering/Components/PointLight.hpp>

#include <d3d11.h>

#define MAX_POINTLIGHTS 7u

class IBuffer;
class ICommandList;
class DeviceDX11;
class RenderableHandler;
class Texture;
class TransformHandler;
class VPHandler;
class Window;

class MeshRenderer : public Renderer
{
public:
    MeshRenderer(ECSCore* pECS, DeviceDX11* pDevice, Window* pWindow);
    ~MeshRenderer();

    bool init() override final;
    void recordCommands() override final;
    void executeCommands() override final;

private:
    struct PointLightBuffer {
        DirectX::XMFLOAT3 Position;
        float RadiusReciprocal;
        DirectX::XMFLOAT3 Light;
        float Padding;
    };

    struct PerObjectMatrices {
        DirectX::XMFLOAT4X4 WVP, World;
    };

    struct PerFrameBuffer {
        PointLightBuffer PointLights[MAX_POINTLIGHTS];
        DirectX::XMFLOAT3 CameraPosition;
        uint32_t NumLights;
        DirectX::XMFLOAT4 Padding;
    };

private:
    IDVector m_Renderables;
    IDVector m_Camera;
    IDVector m_PointLights;

    ICommandList* m_pCommandList;

    RenderableHandler* m_pRenderableHandler;
    TransformHandler* m_pTransformHandler;
    VPHandler* m_pVPHandler;
    LightHandler* m_pLightHandler;

    /* Mesh input layout */
    ID3D11InputLayout* m_pMeshInputLayout;

    /* Cbuffers */
    // WVP and W matrices
    IBuffer* m_pPerObjectMatrices;
    IBuffer* m_pMaterialBuffer;
    // Contains pointlights, camera position and number of lights
    IBuffer* m_pPointLightBuffer;

    /* Samplers */
    ID3D11SamplerState *const* m_ppAniSampler;

    /* Render targets */
    Texture* m_pRenderTarget;
    Texture* m_pDepthStencil;

    ID3D11RasterizerState* m_RsState;

    D3D11_VIEWPORT m_Viewport;
    unsigned int m_BackbufferWidth, m_BackbufferHeight;
};
