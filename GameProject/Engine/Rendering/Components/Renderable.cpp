#include "Renderable.hpp"

#include <Engine/ECS/SystemSubscriber.hpp>
#include <Engine/Rendering/AssetLoaders/ModelLoader.hpp>
#include <Engine/Utils/Logger.hpp>

RenderableHandler::RenderableHandler(SystemSubscriber* systemSubscriber)
    :ComponentHandler({tid_renderable}, systemSubscriber, std::type_index(typeid(RenderableHandler)))
{
    std::vector<ComponentRegistration> compRegs = {
        {tid_renderable, [this](Entity entity) {return renderables.hasElement(entity);}, &renderables.getIDs()}
    };

    this->registerHandler(&compRegs);

    std::type_index tid_shaderHandler = std::type_index(typeid(ShaderHandler));
    std::type_index tid_modelLoader = std::type_index(typeid(ModelLoader));

    this->shaderHandler = static_cast<ShaderHandler*>(systemSubscriber->getComponentHandler(tid_shaderHandler));
    this->modelLoader = static_cast<ModelLoader*>(systemSubscriber->getComponentHandler(tid_modelLoader));
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
    this->registerComponent(tid_renderable, entity);
    return true;
}
