#pragma once

#define NOMINMAX

#include <Engine/ECS/ComponentSubscriptionRequest.hpp>

class ComponentHandler;
class Device;
class ECSCore;
class Renderer;
class RenderingHandler;

struct RendererRegistration {
    ComponentSubscriberRegistration SubscriberRegistration;
    Renderer* pRenderer;
};

class Renderer
{
public:
    Renderer(ECSCore* pECS, Device* pDevice, RenderingHandler* pRenderingHandler);
    ~Renderer();

    virtual bool init() = 0;

    virtual void updateBuffers() = 0;
    virtual void recordCommands() = 0;
    virtual void executeCommands(ICommandList* pPrimaryCommandList) = 0;

    void setComponentSubscriptionID(size_t ID) { m_ComponentSubscriptionID = ID; }

protected:
    void registerRenderer(const RendererRegistration& rendererRegistration);
    ComponentHandler* getComponentHandler(const std::type_index& handlerType);

protected:
    Device* m_pDevice;
    RenderingHandler* m_pRenderingHandler;

private:
    ECSCore* m_pECS;

    size_t m_ComponentSubscriptionID;
};
