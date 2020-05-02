#pragma once

#define NOMINMAX

#include <Engine/ECS/Renderer.hpp>
#include <Engine/Utils/IDVector.hpp>

#include <d3d11.h>
#include <wrl/client.h>

class BufferDX11;
class DeviceDX11;
class ShaderHandler;
class UIHandler;
class Window;
struct Program;

class UIRenderer : public Renderer
{
public:
    UIRenderer(ECSCore* pECS, DeviceDX11* pDevice, Window* pWindow);
    ~UIRenderer();

    virtual bool init() override;
    virtual void recordCommands() override;
    virtual bool executeCommands() override;

private:
    IDVector m_Panels;

    ID3D11DeviceContext* m_pCommandBuffer;

    UIHandler* m_pUIHandler;

    ShaderHandler* m_pShaderHandler;

    Program* m_pUIProgram;

    BufferDX11* m_pQuad;

    /* Render targets */
    ID3D11RenderTargetView* m_pRenderTarget;
    ID3D11DepthStencilView* m_pDepthStencilView;

    // Constant buffer
    Microsoft::WRL::ComPtr<ID3D11Buffer> m_PerPanelBuffer;

    /* Samplers */
    ID3D11SamplerState *const* m_ppAniSampler;

    D3D11_VIEWPORT m_Viewport;
    unsigned int m_BackbufferWidth, m_BackbufferHeight;
};
