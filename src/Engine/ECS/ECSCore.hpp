#pragma once

#include <Engine/ECS/ComponentHandler.hpp>
#include <Engine/ECS/ComponentSubscriber.hpp>
#include <Engine/ECS/ECSBooter.hpp>
#include <Engine/ECS/EntityRegistry.hpp>
#include <Engine/ECS/JobScheduler.hpp>
#include <Engine/ECS/Renderer.hpp>
#include <Engine/ECS/System.hpp>
#include <Engine/ECS/SystemRegistry.hpp>
#include <Engine/Utils/IDGenerator.hpp>

#include <vector>

class ECSCore
{
public:
    ECSCore();
    ~ECSCore() = default;

    ECSCore(const ECSCore& other) = delete;
    void operator=(const ECSCore& other) = delete;

    void update(float dt);

    inline Entity createEntity() { return m_EntityRegistry.createEntity(); }

    void scheduleJobASAP(const Job& job);
    void scheduleJobPostFrame(const Job& job);

    void enqueueComponentHandlerRegistration(const ComponentHandlerRegistration& handlerRegistration);
    void enqueueSystemRegistration(const SystemRegistration& systemRegistration);
    void enqueueRendererRegistration(const RendererRegistration& rendererRegistration);

    // Registers and initializes component handlers and systems
    void performRegistrations();

    // Enqueues an entity deletion, performed during maintenance
    void deleteEntityDelayed(Entity entity);
    void performDeletions();

    void addRegistryPage();
    void deregisterTopRegistryPage();
    void deleteTopRegistryPage();
    void reinstateTopRegistryPage();

    void componentAdded(Entity entity, std::type_index componentType);
    void componentDeleted(Entity entity, std::type_index componentType);

    inline ComponentSubscriber* getComponentSubscriber()    { return &m_ComponentSubscriber; }
    inline SystemRegistry* getSystemRegistry()              { return &m_SystemRegistry; }

private:
    EntityRegistry m_EntityRegistry;
    ComponentSubscriber m_ComponentSubscriber;
    SystemRegistry m_SystemRegistry;
    JobScheduler m_JobScheduler;
    ECSBooter m_ECSBooter;

    std::vector<Entity> m_EntitiesToDelete;

    // DeltaTime is the time between frames. The typical 'dt' that is passed to update()
    float m_DeltaTime;
};
