#pragma once

#define NOMINMAX

#include <Engine/ECS/Renderer.hpp>
#include <wrl/client.h>
#include <d3d11.h>

class Display;
class ShaderHandler;
class UIHandler;
struct Program;

class UIRenderer : public Renderer
{
public:
    UIRenderer(ECSCore* pECS, Display* pDisplay);
    ~UIRenderer();

    virtual bool init() override;
    virtual void recordCommands() override;
    virtual bool executeCommands() override;

private:
    IDVector<Entity> m_Panels;

    ID3D11DeviceContext* m_pCommandBuffer;

    UIHandler* m_pUIHandler;

    ShaderHandler* m_pShaderHandler;

    Program* m_pUIProgram;

    Microsoft::WRL::ComPtr<ID3D11Buffer> m_Quad;

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
