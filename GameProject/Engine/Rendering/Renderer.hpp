#pragma once

#include <Engine/Rendering/ShaderHandler.hpp>

class Renderer // : public System
{
public:
    Renderer(ID3D11Device* device);
    ~Renderer();

private:
    ShaderHandler shaderHandler;
};
