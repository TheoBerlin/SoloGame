#pragma once

#include <Engine/ECS/EntitySubscriber.hpp>

class ComponentHandler;
class Device;
class ECSCore;
class Renderer;
class RenderingHandler;

class Renderer : private EntitySubscriber
{
public:
    Renderer(ECSCore* pECS, Device* pDevice, RenderingHandler* pRenderingHandler);
    virtual ~Renderer() = default;

    virtual bool init() = 0;

    virtual void updateBuffers() = 0;
    virtual void recordCommands() = 0;
    virtual void executeCommands(ICommandList* pPrimaryCommandList) = 0;

protected:
    void registerRenderer(const EntitySubscriberRegistration& subscriberRegistration);
    ComponentHandler* getComponentHandler(const std::type_index& handlerType);

protected:
    Device* m_pDevice;
    RenderingHandler* m_pRenderingHandler;

private:
    ECSCore* m_pECS;
};
