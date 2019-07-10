#include "Renderer.hpp"

#include <Engine/ECS/ECSInterface.hpp>
#include <Engine/ECS/SystemSubscriber.hpp>
#include <Engine/Rendering/Components/Renderable.hpp>
#include <Engine/Rendering/ShaderHandler.hpp>

Renderer::Renderer(ECSInterface* ecs)
    :System(ecs)
{
    std::type_index tid_shaderHandler = std::type_index(typeid(ShaderHandler));

    this->shaderHandler = static_cast<ShaderHandler*>(ecs->systemSubscriber.getComponentHandler(tid_shaderHandler));

    SystemRegistration sysReg = {
    {
        {{{R, tid_renderable}}, &renderables}
    },
    this};

    this->subscribeToComponents(&sysReg);
}

Renderer::~Renderer()
{}

void Renderer::update(float dt)
{}
