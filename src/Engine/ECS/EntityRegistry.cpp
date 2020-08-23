#include "EntityRegistry.hpp"

#include <Engine/Utils/Logger.hpp>

EntityRegistry::EntityRegistry()
{
    addPage();
}

EntityRegistry::~EntityRegistry()
{}

void EntityRegistry::registerComponentType(Entity entity, std::type_index componentType)
{
    EntityRegistryPage& topPage = m_EntityPages.top();

    if (!topPage.hasElement(entity)) {
        // Initialize a new set
        topPage.push_back({componentType}, entity);
    } else {
        // Add the component type to the set
        topPage.indexID(entity).insert(componentType);
    }
}

void EntityRegistry::deregisterComponentType(Entity entity, std::type_index componentType)
{
    EntityRegistryPage& topPage = m_EntityPages.top();
    if (!topPage.hasElement(entity)) {
        LOG_WARNINGF("Attempted to deregister a component type (%s) from an unregistered entity: %ld", componentType.name(), entity);
    } else {
        topPage.indexID(entity).erase(componentType);
    }
}

bool EntityRegistry::entityHasTypes(Entity entity, const std::vector<std::type_index>& queryTypes) const
{
    const EntityRegistryPage& topPage = m_EntityPages.top();
    const std::unordered_set<std::type_index>& entityTypes = topPage.indexID(entity);

    for (const std::type_index& type : queryTypes) {
        auto got = entityTypes.find(type);
        if (got == entityTypes.end()) {
            return false;
        }
    }

    return true;
}

Entity EntityRegistry::createEntity()
{
    Entity newEntity = m_EntityIDGen.genID();

    EntityRegistryPage& topPage = m_EntityPages.top();
    topPage.push_back({}, newEntity);

    return newEntity;
}

void EntityRegistry::deregisterEntity(Entity entity)
{
    EntityRegistryPage& topPage = m_EntityPages.top();
    if (!topPage.hasElement(entity)) {
        LOG_WARNINGF("Attempted to deregister an unregistered entity: %ld", entity);
        return;
    }

    topPage.pop(entity);
    m_EntityIDGen.popID(entity);
}

void EntityRegistry::addPage()
{
    m_EntityPages.push(EntityRegistryPage());
}

void EntityRegistry::removePage()
{
    EntityRegistryPage& topPage = m_EntityPages.top();
    const std::vector<Entity> entities = topPage.getIDs();
    for (Entity entity : entities) {
        m_EntityIDGen.popID(entity);
    }

    m_EntityPages.pop();
}
