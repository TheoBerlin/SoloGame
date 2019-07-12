#pragma once

#include <Engine/ECS/System.hpp>
#include <d3d11.h>

class ShaderHandler;

class Renderer : public System
{
public:
    Renderer(ECSInterface* ecs, ID3D11Device* device);
    ~Renderer();

    void update(float dt);

private:
    ShaderHandler* shaderHandler;

    IDVector<Entity> renderables;

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
