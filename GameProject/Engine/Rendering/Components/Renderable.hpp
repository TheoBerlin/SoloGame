#pragma once

#define NOMINMAX
#include <Engine/ECS/ComponentHandler.hpp>
#include <Engine/Rendering/ShaderHandler.hpp>
#include <Engine/Utils/IDVector.hpp>
#include <Engine/Utils/ECSUtils.hpp>
#include <string>
#include <typeindex>

class ModelLoader;

struct Model;

struct Renderable {
    Model* model;
    Program* program;
};

const std::type_index tid_renderable = TID(Renderable);

class RenderableHandler : public ComponentHandler
{
public:
    RenderableHandler(ECSCore* pECS);

    virtual bool init() override;

    // Creates a renderable component by loading from file
    bool createRenderable(Entity entity, std::string modelPath, PROGRAM program);
    // Creates a renderable component out of an existing model
    bool createRenderable(Entity entity, Model* model, PROGRAM program);

    IDDVector<Renderable> m_Renderables;

private:
    ShaderHandler* m_pShaderHandler;
    ModelLoader* m_pModelLoader;
};
