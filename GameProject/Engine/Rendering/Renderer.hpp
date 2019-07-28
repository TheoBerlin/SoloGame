#pragma once

#include <Engine/ECS/System.hpp>
#include <d3d11.h>

class ShaderHandler;
class TransformHandler;

class Renderer : public System
{
public:
    Renderer(ECSInterface* ecs, ID3D11Device* device, ID3D11DeviceContext* context);
    ~Renderer();

    void update(float dt);

private:
    RenderableHandler* renderableHandler;
    ShaderHandler* shaderHandler;
    TransformHandler* transformHandler;

    IDVector<Entity> renderables;

    ID3D11DeviceContext* context;

    /* Shader resources */
    /* Cbuffers */
    // WVP and W matrices
    ID3D11Buffer* perObjectMatrices;
    ID3D11Buffer* materialCBuffer;
    ID3D11Buffer* pointLightCBuffer;
    // Contains camera position and number of lights
    ID3D11Buffer* perFramePS;

    /* Samplers */
    ID3D11SamplerState* aniSampler;
};

struct PerObjectMatrices {
    DirectX::XMFLOAT4X4 WVP;
    DirectX::XMFLOAT4X4 world;
};
