#pragma once

#include <Engine/ECS/EntitySubscriber.hpp>

class Device;
class RenderingHandler;

class Renderer : EntitySubscriber
{
public:
    Renderer(Device* pDevice, RenderingHandler* pRenderingHandler);
    virtual ~Renderer() = default;

    virtual bool Init() { return true; }

    virtual void UpdateBuffers() = 0;
    virtual void RecordCommands() = 0;
    virtual void ExecuteCommands(ICommandList* pPrimaryCommandList) = 0;

protected:
    void RegisterRenderer(EntitySubscriberRegistration& subscriberRegistration);

protected:
    Device* m_pDevice;
    RenderingHandler* m_pRenderingHandler;
};
