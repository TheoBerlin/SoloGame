#include "EntityRegistry.hpp"

#include <mutex>

EntityRegistry::EntityRegistry()
{
	AddPage();
}

void EntityRegistry::RegisterComponentType(Entity entity, const ComponentType* pComponentType)
{
	std::scoped_lock<std::mutex> lock(m_Lock);

	EntityRegistryPage& topPage = m_EntityPages.top();
	if (!topPage.HasElement(entity)) {
		// Initialize a new set
		topPage.push_back({ pComponentType }, entity);
	} else {
		// Add the component type to the set
		topPage.IndexID(entity).insert(pComponentType);
	}
}

void EntityRegistry::DeregisterComponentType(Entity entity, const ComponentType* pComponentType)
{
	std::scoped_lock<std::mutex> lock(m_Lock);

	EntityRegistryPage& topPage = m_EntityPages.top();
	if (!topPage.HasElement(entity)) {
		LOG_WARNINGF("Attempted to deregister a component type (%s) from an unregistered entity: %u",
			pComponentType->Name(), entity);
	} else {
		topPage.IndexID(entity).erase(pComponentType);
	}
}

bool EntityRegistry::EntityHasAllowedTypes(Entity entity, const std::vector<const ComponentType*>& allowedTypes, const std::vector<const ComponentType*>& disallowedTypes) const
{
	std::scoped_lock<std::mutex> lock(m_Lock);

	const EntityRegistryPage& topPage = m_EntityPages.top();
	const std::unordered_set<const ComponentType*>& entityTypes = topPage.IndexID(entity);

	const bool hasDisallowedType = std::any_of(disallowedTypes.begin(), disallowedTypes.end(), [entityTypes](const ComponentType* pType) {
		return entityTypes.contains(pType);
	});

	if (hasDisallowedType) {
		return false;
	}

	return std::none_of(allowedTypes.begin(), allowedTypes.end(), [entityTypes](const ComponentType* pType) {
		return !entityTypes.contains(pType);
	});
}

bool EntityRegistry::EntityHasAllTypes(Entity entity, const std::vector<const ComponentType*>& types) const
{
	std::scoped_lock<std::mutex> lock(m_Lock);

	const EntityRegistryPage& topPage = m_EntityPages.top();
	const std::unordered_set<const ComponentType*>& entityTypes = topPage.IndexID(entity);

	// Check that none of the entity types are missing
	return std::none_of(types.begin(), types.end(), [entityTypes](const ComponentType* pType) {
		return !entityTypes.contains(pType);
	});
}

bool EntityRegistry::EntityHasAnyOfTypes(Entity entity, const std::vector<const ComponentType*>& types) const
{
	std::scoped_lock<std::mutex> lock(m_Lock);

	const EntityRegistryPage& topPage = m_EntityPages.top();
	const std::unordered_set<const ComponentType*>& entityTypes = topPage.IndexID(entity);

	return std::any_of(types.begin(), types.end(), [entityTypes](const ComponentType* pType) {
		return entityTypes.contains(pType);
	});
}

Entity EntityRegistry::CreateEntity()
{
	std::scoped_lock<std::mutex> lock(m_Lock);

	const Entity newEntity = m_EntityIDGen.GenID();

	EntityRegistryPage& topPage = m_EntityPages.top();
	topPage.push_back({}, newEntity);

	return newEntity;
}

void EntityRegistry::DeregisterEntity(Entity entity)
{
	std::scoped_lock<std::mutex> lock(m_Lock);

	EntityRegistryPage& topPage = m_EntityPages.top();
	if (!topPage.HasElement(entity)) {
		return;
	}

	topPage.Pop(entity);
	m_EntityIDGen.PopID(entity);
}

void EntityRegistry::AddPage()
{
	m_EntityPages.push({});
}

void EntityRegistry::RemovePage()
{
	EntityRegistryPage& topPage = m_EntityPages.top();
	const std::vector<Entity> entities = topPage.GetIDs();
	for (Entity entity : entities) {
		m_EntityIDGen.PopID(entity);
	}

	m_EntityPages.pop();
}
