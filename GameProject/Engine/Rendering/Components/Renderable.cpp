#include "Renderable.hpp"

#include <Engine/ECS/SystemSubscriber.hpp>
#include <Engine/Rendering/AssetLoaders/ModelLoader.hpp>

RenderableHandler::RenderableHandler(SystemSubscriber* systemSubscriber)
    :ComponentHandler({tid_renderable}, systemSubscriber, std::type_index(typeid(RenderableHandler)))
{
    std::vector<ComponentRegistration> compRegs = {
        {tid_renderable, [this](Entity entity) {return renderables.hasElement(entity);}, &renderables.getIDs()}
    };

    this->registerHandler(&compRegs);

    std::type_index tid_shaderHandler = std::type_index(typeid(ShaderHandler));

    this->shaderHandler = static_cast<ShaderHandler*>(systemSubscriber->getComponentHandler(tid_shaderHandler));
}

bool RenderableHandler::createRenderable(std::string modelPath, PROGRAM program)
{
    Renderable renderable;

    renderable.program = shaderHandler->getProgram(program);

    renderable.model = ModelLoader::loadModel(modelPath, {DIFFUSE});

    return renderable.program && renderable.model;
}
