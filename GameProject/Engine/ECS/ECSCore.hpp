#pragma once

#include <Engine/ECS/ComponentHandler.hpp>
#include <Engine/ECS/ComponentSubscriber.hpp>
#include <Engine/ECS/ECSBooter.hpp>
#include <Engine/ECS/EntityRegistry.hpp>
#include <Engine/ECS/Renderer.hpp>
#include <Engine/ECS/System.hpp>
#include <Engine/ECS/SystemUpdater.hpp>
#include <Engine/Utils/IDGenerator.hpp>

#include <vector>

class ECSCore
{
public:
    ECSCore();
    ~ECSCore();

    ECSCore(const ECSCore& other) = delete;
    void operator=(const ECSCore& other) = delete;

    void update(float dt);

    Entity createEntity();

    void enqueueComponentHandlerRegistration(const ComponentHandlerRegistration& handlerRegistration);
    void enqueueSystemRegistration(const SystemRegistration& systemRegistration);
    void enqueueRendererRegistration(const RendererRegistration& rendererRegistration);

    // Registers and initializes component handlers and systems
    void performRegistrations();

    // Enqueues an entity deletion, performed during maintenance
    void deleteEntityDelayed(Entity entity);
    void performDeletions();

    ComponentSubscriber* getComponentSubscriber() { return &m_ComponentSubscriber; }
    SystemUpdater* getSystemUpdater() { return &m_SystemUpdater; }
    EntityRegistry* getEntityRegistry() { return &m_EntityRegistry; }

    void addRegistryPage();
    void deregisterTopRegistryPage();
    void deleteTopRegistryPage();
    void reinstateTopRegistryPage();

    void componentAdded(Entity entity, std::type_index componentType);
    void componentDeleted(Entity entity, std::type_index componentType);

private:
    EntityRegistry m_EntityRegistry;
    ComponentSubscriber m_ComponentSubscriber;
    SystemUpdater m_SystemUpdater;
    ECSBooter m_ECSBooter;

    std::vector<Entity> m_EntitiesToDelete;
};
