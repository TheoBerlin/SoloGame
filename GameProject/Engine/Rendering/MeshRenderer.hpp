#pragma once

#define NOMINMAX
#include <Engine/ECS/Renderer.hpp>
#include <Engine/Rendering/APIAbstractions/Viewport.hpp>
#include <Engine/Rendering/Components/PointLight.hpp>

#include <DirectXMath.h>

#define MAX_POINTLIGHTS 7u

class Device;
class IBuffer;
class ICommandList;
class IDepthStencilState;
class IRasterizerState;
class ISampler;
class RenderableHandler;
class Texture;
class TransformHandler;
class VPHandler;
class Window;

class MeshRenderer : public Renderer
{
public:
    MeshRenderer(ECSCore* pECS, Device* pDevice, Window* pWindow);
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

    /* Uniform buffers */
    // WVP and World matrices
    IBuffer* m_pPerObjectMatrices;
    IBuffer* m_pMaterialBuffer;
    // Contains pointlights, camera position and number of lights
    IBuffer* m_pPointLightBuffer;

    ISampler* m_pAniSampler;

    // Render targets
    Texture* m_pRenderTarget;
    Texture* m_pDepthStencil;

    IRasterizerState* m_pRasterizerState;
    IDepthStencilState* m_pDepthStencilState;

    Viewport m_Viewport;
    unsigned int m_BackbufferWidth, m_BackbufferHeight;
};
