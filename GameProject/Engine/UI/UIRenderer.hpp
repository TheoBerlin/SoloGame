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
    UIRenderer(ECSCore* pECS, ID3D11DeviceContext* context, ID3D11Device* device, ID3D11RenderTargetView* rtv, ID3D11DepthStencilView* dsv);
    ~UIRenderer();

    void update(float dt);

private:
    IDVector<Entity> panels;

    UIHandler* UIhandler;

    ShaderHandler* shaderHandler;

    Program* UIProgram;

    ID3D11DeviceContext* context;

    Microsoft::WRL::ComPtr<ID3D11Buffer> quad;

    /* Render targets */
    ID3D11RenderTargetView* renderTarget;
    ID3D11DepthStencilView* depthStencilView;

    // Constant buffer
    Microsoft::WRL::ComPtr<ID3D11Buffer> perPanelBuffer;

    /* Samplers */
    ID3D11SamplerState *const* aniSampler;
};
