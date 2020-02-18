#include "Renderable.hpp"

#include <Engine/ECS/ECSCore.hpp>
#include <Engine/ECS/SystemSubscriber.hpp>
#include <Engine/Rendering/AssetLoaders/ModelLoader.hpp>
#include <Engine/Utils/Logger.hpp>

RenderableHandler::RenderableHandler(ECSCore* pECS)
    :ComponentHandler({tid_renderable}, pECS, std::type_index(typeid(RenderableHandler)))
{
    std::vector<ComponentRegistration> compRegs = {
        {tid_renderable, &renderables}
    };

    this->registerHandler(&compRegs);

    std::type_index tid_shaderHandler = std::type_index(typeid(ShaderHandler));
    std::type_index tid_modelLoader = std::type_index(typeid(ModelLoader));

    this->shaderHandler = static_cast<ShaderHandler*>(pECS->getSystemSubscriber()->getComponentHandler(tid_shaderHandler));
    this->modelLoader = static_cast<ModelLoader*>(pECS->getSystemSubscriber()->getComponentHandler(tid_modelLoader));
}

bool RenderableHandler::createRenderable(Entity entity, std::string modelPath, PROGRAM program)
{
    Renderable renderable;

    renderable.program = shaderHandler->getProgram(program);
    renderable.model = modelLoader->loadModel(modelPath);

    if (!renderable.program || !renderable.model) {
        Logger::LOG_WARNING("Failed to create renderable component for entity: %d", entity);
        return false;
    }

    renderables.push_back(renderable, entity);
    this->registerComponent(entity, tid_renderable);
    return true;
}

bool RenderableHandler::createRenderable(Entity entity, Model* model, PROGRAM program)
{
    Renderable renderable;

    renderable.program = shaderHandler->getProgram(program);
    renderable.model = model;

    if (!renderable.program || !renderable.model) {
        Logger::LOG_WARNING("Failed to create renderable component for entity: %d", entity);
        return false;
    }

    renderables.push_back(renderable, entity);
    this->registerComponent(entity, tid_renderable);
    return true;
}
