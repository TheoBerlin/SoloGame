#include "ECSCore.hpp"

ECSCore* ECSCore::s_pInstance = nullptr;

ECSCore::ECSCore() :
	m_EntityPublisher(&m_ComponentStorage, &m_EntityRegistry)
{}

void ECSCore::Update(float deltaTime)
{
	m_DeltaTime = deltaTime;
	PerformComponentRegistrations();
	PerformComponentDeletions();
	PerformEntityDeletions();
	m_JobScheduler.Update(deltaTime);
	m_ComponentStorage.ResetDirtyFlags();
}

IComponentArray* ECSCore::GetComponentArray(const ComponentType* pComponentType)
{
	return m_ComponentStorage.GetComponentArray(pComponentType);
}

const IComponentArray* ECSCore::GetComponentArray(const ComponentType* pComponentType) const
{
	return m_ComponentStorage.GetComponentArray(pComponentType);
}

void ECSCore::RemoveEntity(Entity entity)
{
	std::scoped_lock<std::mutex> lock(m_LockRemoveEntity);
	m_EntitiesToDelete.insert(entity);
}

void ECSCore::ScheduleJobASAP(const Job& job)
{
	m_JobScheduler.ScheduleJobASAP(job);
}

void ECSCore::ScheduleJobPostFrame(const Job& job)
{
	m_JobScheduler.ScheduleJob(job, LAST_PHASE + 1u);
}

void ECSCore::AddRegistryPage()
{
	m_EntityRegistry.AddPage();
}

void ECSCore::DeregisterTopRegistryPage()
{
	const EntityRegistryPage& page = m_EntityRegistry.GetTopRegistryPage();

	const auto& entityComponentSets = page.GetVec();
	const std::vector<Entity>& entities = page.GetIDs();

	for (uint32_t entityIdx = 0; entityIdx < entities.size(); entityIdx++)
	{
		const std::unordered_set<const ComponentType*>& typeSet = entityComponentSets[entityIdx];

		for (const ComponentType* pComponentType : typeSet)
		{
			// Deregister entity's components from systems
			m_EntityPublisher.UnpublishComponent(entities[entityIdx], pComponentType);
		}
	}
}

void ECSCore::DeleteTopRegistryPage()
{
	const EntityRegistryPage& page = m_EntityRegistry.GetTopRegistryPage();
	const auto& entityComponentSets = page.GetVec();
	const std::vector<Entity>& entities = page.GetIDs();
	std::vector<const ComponentType*> componentTypes;

	const uint32_t entityCount = (uint32_t)entities.size();
	for (uint32_t entityNr = 0; entityNr < entityCount; entityNr++) {
		const Entity entity = entities[entityNr];
		const std::unordered_set<const ComponentType*>& typeSet = entityComponentSets[entityNr];
		componentTypes.assign(typeSet.begin(), typeSet.end());

		for (const ComponentType* pComponentType : componentTypes) {
			m_EntityRegistry.DeregisterComponentType(entity, pComponentType);
		}

		for (const ComponentType* pComponentType : componentTypes) {
			m_EntityPublisher.UnpublishComponent(entity, pComponentType);
			m_ComponentStorage.DeleteComponent(entity, pComponentType);
		}
	}

	m_EntityRegistry.RemovePage();
}

void ECSCore::ReinstateTopRegistryPage()
{
	const EntityRegistryPage& page = m_EntityRegistry.GetTopRegistryPage();

	const auto& entityComponentSets = page.GetVec();
	const std::vector<Entity>& entities = page.GetIDs();

	for (uint32_t entityIdx = 0; entityIdx < entities.size(); entityIdx++) {
		const std::unordered_set<const ComponentType*>& typeSet = entityComponentSets[entityIdx];

		for (const ComponentType* pComponentType : typeSet) {
			m_EntityPublisher.PublishComponent(entities[entityIdx], pComponentType);
		}
	}
}

uint32_t ECSCore::SerializeEntity(Entity entity, const std::vector<const ComponentType*>& componentsFilter, uint8_t* pBuffer, uint32_t bufferSize) const
{
	/*	EntitySerializationHeader is written to the beginning of the buffer. This is done last, when the size of
		the serialization is known. */
	uint8_t* pHeaderPosition = pBuffer;

	uint32_t remainingSize = bufferSize;
	constexpr const uint32_t headerSize = sizeof(EntitySerializationHeader);
	const bool hasRoomForHeader = bufferSize >= headerSize;
	if (hasRoomForHeader) {
		pBuffer			+= headerSize;
		remainingSize	-= headerSize;
	}

	// Serialize all components
	uint32_t requiredTotalSize = headerSize;
	uint32_t serializedComponentsCount = 0u;
	for (const ComponentType* pComponentType : componentsFilter) {
		const IComponentArray* pComponentArray = m_ComponentStorage.GetComponentArray(pComponentType);
		if (pComponentArray && !pComponentArray->HasComponent(entity)) {
			LOG_WARNINGF("Attempted to serialize a component type which entity %d does not have: %s", entity, pComponentType->Name());
			continue;
		}

		const uint32_t requiredComponentSize = m_ComponentStorage.SerializeComponent(entity, pComponentType, pBuffer, remainingSize);
		requiredTotalSize += requiredComponentSize;
		if (requiredComponentSize <= remainingSize) {
			pBuffer += requiredComponentSize;
			remainingSize -= requiredComponentSize;
			++serializedComponentsCount;
		}
	}

	// Finalize the serialization by writing the header
	if (hasRoomForHeader) {
		const EntitySerializationHeader header = {
			.TotalSerializationSize	= requiredTotalSize,
			.Entity					= entity,
			.ComponentCount			= serializedComponentsCount
		};

		memcpy(pHeaderPosition, &header, headerSize);
	}

	return requiredTotalSize;
}

bool ECSCore::DeserializeEntity(const uint8_t* pBuffer)
{
	constexpr const uint32_t entityHeaderSize = sizeof(EntitySerializationHeader);
	EntitySerializationHeader entityHeader;
	memcpy(&entityHeader, pBuffer, entityHeaderSize);
	pBuffer += entityHeaderSize;

	ASSERT_MSG(m_EntityRegistry.GetTopRegistryPage().HasElement(entityHeader.Entity), "Attempted to deserialize unknown entity: %d", entityHeader.Entity);

	// Deserialize each component. If the entity already has the component, update its data. Otherwise, create it.
	const uint32_t componentCount = entityHeader.ComponentCount;
	bool success = true;
	for (uint32_t componentIdx = 0u; componentIdx < componentCount; ++componentIdx) {
		constexpr const uint32_t componentHeaderSize = sizeof(ComponentSerializationHeader);
		ComponentSerializationHeader componentHeader;
		memcpy(&componentHeader, pBuffer, componentHeaderSize);
		pBuffer += componentHeaderSize;

		const ComponentType* pComponentType = m_ComponentStorage.GetComponentType(componentHeader.TypeHash);
		if (!pComponentType) {
			LOG_WARNINGF("Attempted to deserialize an unregistered component type, hash: %d", componentHeader.TypeHash);
			success = false;
			continue;
		}

		bool entityHadComponent = false;
		const uint32_t componentDataSize = componentHeader.TotalSerializationSize - componentHeaderSize;
		success = m_ComponentStorage.DeserializeComponent(entityHeader.Entity, pComponentType, componentDataSize, pBuffer, entityHadComponent) && success;

		if (!entityHadComponent) {
			m_ComponentsToRegister.push_back({entityHeader.Entity, pComponentType});
		}
	}

	return success;
}

void ECSCore::PerformComponentRegistrations()
{
	// Register all components first, then publish them
	for (const std::pair<Entity, const ComponentType*>& component : m_ComponentsToRegister) {
		m_EntityRegistry.RegisterComponentType(component.first, component.second);
	}

	for (const std::pair<Entity, const ComponentType*>& component : m_ComponentsToRegister) {
		m_EntityPublisher.PublishComponent(component.first, component.second);
	}

	m_ComponentsToRegister.shrink_to_fit();
	m_ComponentsToRegister.clear();
}

void ECSCore::PerformComponentDeletions()
{
	for (const std::pair<Entity, const ComponentType*>& component : m_ComponentsToDelete) {
		if (DeleteComponent(component.first, component.second)) {
			// If the entity has no more components, delete it
			const std::unordered_set<const ComponentType*>& componentTypes = m_EntityRegistry.GetTopRegistryPage().IndexID(component.first);
			if (componentTypes.empty()) {
				m_EntityRegistry.DeregisterEntity(component.first);
			}
		}
	}

	m_ComponentsToDelete.shrink_to_fit();
	m_ComponentsToDelete.clear();
}

void ECSCore::PerformEntityDeletions()
{
	const EntityRegistryPage& registryPage = m_EntityRegistry.GetTopRegistryPage();
	/*	The component types to delete of each entity. It is a copy of the entity's set of component types in
		the entity registry. Copying the set is necessary as the set is popped each time it is iterated. */
	std::vector<const ComponentType*> componentTypes;

	for (Entity entity : m_EntitiesToDelete) {
		if (registryPage.HasElement(entity)) {
			// Delete every component belonging to the entity
			const std::unordered_set<const ComponentType*>& componentTypesSet = registryPage.IndexID(entity);
			componentTypes.assign(componentTypesSet.begin(), componentTypesSet.end());

			for (const ComponentType* pComponentType : componentTypes) {
				m_EntityRegistry.DeregisterComponentType(entity, pComponentType);
			}

			for (const ComponentType* pComponentType : componentTypes) {
				m_EntityPublisher.UnpublishComponent(entity, pComponentType);
				m_ComponentStorage.DeleteComponent(entity, pComponentType);
			}

			// Free the entity ID
			m_EntityRegistry.DeregisterEntity(entity);
		}
	}

	m_EntitiesToDelete.clear();
}

bool ECSCore::DeleteComponent(Entity entity, const ComponentType* pComponentType)
{
	m_EntityRegistry.DeregisterComponentType(entity, pComponentType);
	m_EntityPublisher.UnpublishComponent(entity, pComponentType);
	return m_ComponentStorage.DeleteComponent(entity, pComponentType);
}
