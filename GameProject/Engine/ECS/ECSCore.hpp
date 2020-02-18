#pragma once

#include <Engine/ECS/EntityRegistry.hpp>
#include <Engine/ECS/SystemSubscriber.hpp>
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

    Entity createEntity();

    // Enqueues an entity deletion, performed during maintenance
    void deleteEntityDelayed(Entity entity);
    void performMaintenance();

    SystemSubscriber* getSystemSubscriber() { return &m_SystemSubscriber; }
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
    SystemSubscriber m_SystemSubscriber;
    SystemUpdater m_SystemUpdater;

    std::vector<Entity> m_EntitiesToDelete;
};
