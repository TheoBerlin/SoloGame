#pragma once

#include <Engine/ECS/System.hpp>

class ShaderHandler;

class Renderer : public System
{
public:
    Renderer(ECSInterface* ecs);
    ~Renderer();

    void update(float dt);

private:
    ShaderHandler* shaderHandler;

    IDVector<Entity> renderables;
};
