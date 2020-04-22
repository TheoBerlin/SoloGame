#include "Renderable.hpp"

#include <Engine/ECS/ECSCore.hpp>
#include <Engine/ECS/ComponentSubscriber.hpp>
#include <Engine/Rendering/AssetLoaders/ModelLoader.hpp>
#include <Engine/Utils/Logger.hpp>

RenderableHandler::RenderableHandler(ECSCore* pECS)
    :ComponentHandler(pECS, TID(RenderableHandler))
{
    ComponentHandlerRegistration handlerReg = {};
    handlerReg.pComponentHandler = this;
    handlerReg.ComponentRegistrations = {
        {g_TIDRenderable, &m_Renderables}
    };
    handlerReg.HandlerDependencies = {
        TID(ShaderHandler),
        TID(ModelLoader)
    };

    this->registerHandler(handlerReg);
}

bool RenderableHandler::init()
{
    m_pShaderHandler = static_cast<ShaderHandler*>(m_pECS->getComponentSubscriber()->getComponentHandler(TID(ShaderHandler)));
    m_pModelLoader = static_cast<ModelLoader*>(m_pECS->getComponentSubscriber()->getComponentHandler(TID(ModelLoader)));

    return m_pShaderHandler && m_pModelLoader;
}

bool RenderableHandler::createRenderable(Entity entity, std::string modelPath, PROGRAM program)
{
    Renderable renderable;

    renderable.program = m_pShaderHandler->getProgram(program);
    renderable.model = m_pModelLoader->loadModel(modelPath);

    if (!renderable.program || !renderable.model) {
        LOG_WARNING("Failed to create renderable component for entity: %d", entity);
        return false;
    }

    m_Renderables.push_back(renderable, entity);
    registerComponent(entity, g_TIDRenderable);
    return true;
}

bool RenderableHandler::createRenderable(Entity entity, Model* model, PROGRAM program)
{
    Renderable renderable;

    renderable.program = m_pShaderHandler->getProgram(program);
    renderable.model = model;

    if (!renderable.program || !renderable.model) {
        LOG_WARNING("Failed to create renderable component for entity: %d", entity);
        return false;
    }

    m_Renderables.push_back(renderable, entity);
    registerComponent(entity, g_TIDRenderable);
    return true;
}
