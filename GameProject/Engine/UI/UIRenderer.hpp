#pragma once

#define NOMINMAX

#include <Engine/ECS/System.hpp>
#include <wrl/client.h>
#include <d3d11.h>

class ShaderHandler;
class UIHandler;
struct Program;

class UIRenderer : public System
{
public:
    UIRenderer(ECSCore* pECS, ID3D11DeviceContext* pContext, ID3D11Device* pDevice, ID3D11RenderTargetView* pRTV, ID3D11DepthStencilView* pDSV);
    ~UIRenderer();

    virtual bool init() override;

    void update(float dt);

private:
    IDVector<Entity> m_Panels;

    UIHandler* m_pUIHandler;

    ShaderHandler* m_pShaderHandler;

    Program* m_pUIProgram;

    ID3D11Device* m_pDevice;
    ID3D11DeviceContext* m_pContext;

    Microsoft::WRL::ComPtr<ID3D11Buffer> m_Quad;

    /* Render targets */
    ID3D11RenderTargetView* m_pRenderTarget;
    ID3D11DepthStencilView* m_pDepthStencilView;

    // Constant buffer
    Microsoft::WRL::ComPtr<ID3D11Buffer> m_PerPanelBuffer;

    /* Samplers */
    ID3D11SamplerState *const* m_ppAniSampler;
};
