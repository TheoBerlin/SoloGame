#pragma once

#define NOMINMAX

#include <Engine/ECS/Renderer.hpp>
#include <Engine/Utils/IDVector.hpp>

#include <d3d11.h>

class Device;
class IBuffer;
class ICommandList;
class IRasterizerState;
class ShaderHandler;
class Texture;
class UIHandler;
class Window;
struct Program;

class UIRenderer : public Renderer
{
public:
    UIRenderer(ECSCore* pECS, Device* pDevice, Window* pWindow);
    ~UIRenderer();

    bool init() override final;
    void recordCommands() override final;
    void executeCommands() override final;

private:
    IDVector m_Panels;

    ICommandList* m_pCommandList;

    UIHandler* m_pUIHandler;

    ShaderHandler* m_pShaderHandler;

    Program* m_pUIProgram;

    // Vertex buffer
    IBuffer* m_pQuad;

    Texture* m_pRenderTarget;
    Texture* m_pDepthStencil;
    IBuffer* m_pPerPanelBuffer;
    ID3D11SamplerState *const* m_ppAniSampler;
    IRasterizerState* m_pRasterizerState;

    D3D11_VIEWPORT m_Viewport;
    unsigned int m_BackbufferWidth, m_BackbufferHeight;
};
