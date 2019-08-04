#pragma once

#define NOMINMAX
#include <Engine/ECS/ComponentHandler.hpp>
#include <Engine/Rendering/ShaderHandler.hpp>
#include <Engine/Utils/IDVector.hpp>
#include <typeindex>

class ModelLoader;

struct Model;

struct Renderable {
    Model* model;
    Program* program;
};

const std::type_index tid_renderable = std::type_index(typeid(Renderable));

class RenderableHandler : public ComponentHandler
{
public:
    RenderableHandler(SystemSubscriber* SystemSubscriber);

    bool createRenderable(Entity entity, std::string modelPath, PROGRAM program);

    IDVector<Renderable> renderables;

private:
    ShaderHandler* shaderHandler;
    ModelLoader* modelLoader;
};
