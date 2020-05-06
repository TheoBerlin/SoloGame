#pragma once

#define NOMINMAX

#include <Engine/ECS/ComponentSubscriptionRequest.hpp>

#include <d3d11.h>

class ComponentHandler;
class ECSCore;
class Device;
class Renderer;

struct RendererRegistration {
    ComponentSubscriberRegistration SubscriberRegistration;
    Renderer* pRenderer;
};

class Renderer
{
public:
    Renderer(ECSCore* pECS, Device* pDevice);
    ~Renderer();

    virtual bool init() = 0;
    virtual void recordCommands() = 0;
    virtual void executeCommands() = 0;

    void setComponentSubscriptionID(size_t ID) { m_ComponentSubscriptionID = ID; }

protected:
    void registerRenderer(const RendererRegistration& rendererRegistration);
    ComponentHandler* getComponentHandler(const std::type_index& handlerType);

protected:
    Device* m_pDevice;

private:
    ECSCore* m_pECS;

    ID3D11DeviceContext* m_pImmediateContext;

    size_t m_ComponentSubscriptionID;
};
