#pragma once

#include "Engine/ECS/ComponentStorage.hpp"
#include "Engine/ECS/EntityPublisher.hpp"
#include "Engine/ECS/EntityRegistry.hpp"
#include "Engine/ECS/JobScheduler.hpp"
#include "Engine/Utils/IDGenerator.hpp"

class EntitySubscriber;
class RegularWorker;
class System;

// EntitySerializationHeader is written to the beginning of an entity serialization
#pragma pack(push, 1)
struct EntitySerializationHeader
{
	uint32_t TotalSerializationSize;
	Entity Entity;
	uint32_t ComponentCount;
};
#pragma pack(pop)

class ECSCore
{
public:
	ECSCore();
	~ECSCore() = default;

	ECSCore(const ECSCore& other) = delete;
	void operator=(const ECSCore& other) = delete;

	static void Release();

	void Update(float deltaTime);

	Entity CreateEntity() { return m_EntityRegistry.CreateEntity(); }

	// Add a component to a specific entity.
	template<typename Comp>
	Comp& AddComponent(Entity entity, const Comp& component);

	// Fetch a reference to a component for a specific entity.
	template<typename Comp>
	Comp& GetComponent(Entity entity);

	// Fetch a reference to a component for a specific entity if it exists, returns true if the component exists, otherwise returns false.
	template<typename Comp>
	bool GetComponentIf(Entity entity, Comp** ppComp);

	// Fetch a const reference to a component for a specific entity.
	template<typename Comp>
	const Comp& GetConstComponent(Entity entity) const;

	// Fetch a const reference to a component for a specific entity if it exists, returns true if the component exists, otherwise returns false.
	template<typename Comp>
	bool GetConstComponentIf(Entity entity, const Comp** ppComp) const;

	// Fetch a pointer to an array containing all components of a specific type.
	template<typename Comp>
	ComponentArray<Comp>* GetComponentArray();

	// Fetch a const pointer to an array containing all components of a specific type.
	template<typename Comp>
	const ComponentArray<Comp>* GetComponentArray() const;

	// Fetch a pointer to an array containing all components of a specific type.
	IComponentArray* GetComponentArray(const ComponentType* pComponentType);

	// Fetch a const pointer to an array containing all components of a specific type.
	const IComponentArray* GetComponentArray(const ComponentType* pComponentType) const;

	// RemoveComponent enqueues the removal of a component, which is performed at the end of the current/next frame.
	template<typename Comp>
	void RemoveComponent(Entity entity);

	// RemoveEntity enqueues the removal of an entity, which is performed at the end of the current/next frame.
	void RemoveEntity(Entity entity);

	template <typename Comp>
	void SetComponentOwner(const ComponentOwnership<Comp>& componentOwnership) { m_ComponentStorage.SetComponentOwner(componentOwnership); }
	void UnsetComponentOwner(const ComponentType* pComponentType) { m_ComponentStorage.UnsetComponentOwner(pComponentType); }

	void ScheduleJobASAP(const Job& job);
	void ScheduleJobPostFrame(const Job& job);

	void AddRegistryPage();
	void DeregisterTopRegistryPage();
	void DeleteTopRegistryPage();
	void ReinstateTopRegistryPage();

	/**
	 * Serializes the entity into the following format:
	 * Total Serialization Size			- 4 bytes (includes the size of the header)
	 * Entity ID						- 4 bytes
	 * Component Count					- 4 bytes
	 * [
	 *	Component Serialization Size	- 4 bytes (includes the size of the header)
		* 	Component Type Hash				- 4 bytes
		*	Component Data					- Component Serialization Size
		* ]
		*
		* @param componentsFilter The component types to serialize
		* \return The required of the serialization.
		* If said return value is greater than the provided bufferSize, the serialization failed.
		* Providing a zero bufferSize and nullptr pBuffer is a slow but valid strategy for getting the required buffer size.
	*/
	uint32_t SerializeEntity(Entity entity, const std::vector<const ComponentType*>& componentsFilter, uint8_t* pBuffer, uint32_t bufferSize) const;
	/**
	 * DeserializeEntity uses the entity and component data to add new components and update existing ones.
	 * \return Whether deserializing all components succeeded.
	*/
	bool DeserializeEntity(const uint8_t* pBuffer);

	void PerformComponentRegistrations();
	void PerformComponentDeletions();
	void PerformEntityDeletions();

	float GetDeltaTime() const { return m_DeltaTime; }

public:
	static void SetInstance(ECSCore* pInstance)	{ s_pInstance = pInstance; }
	static ECSCore* GetInstance()				{ return s_pInstance; }

protected:
	friend EntitySubscriber;
	uint32_t SubscribeToEntities(const EntitySubscriberRegistration& subscriberRegistration) { return m_EntityPublisher.SubscribeToEntities(subscriberRegistration); }
	void UnsubscribeFromEntities(uint32_t subscriptionID) { m_EntityPublisher.UnsubscribeFromEntities(subscriptionID); }

	friend RegularWorker;
	uint32_t ScheduleRegularJob(const RegularJob& job, uint32_t phase)		{ return m_JobScheduler.ScheduleRegularJob(job, phase); }
	void DescheduleRegularJob(uint32_t phase, uint32_t jobID)				{ m_JobScheduler.DescheduleRegularJob(phase, jobID); }

private:
	bool DeleteComponent(Entity entity, const ComponentType* pComponentType);

private:
	EntityRegistry m_EntityRegistry;
	EntityPublisher m_EntityPublisher;
	JobScheduler m_JobScheduler;
	ComponentStorage m_ComponentStorage;

	std::unordered_set<Entity> m_EntitiesToDelete;
	std::vector<std::pair<Entity, const ComponentType*>> m_ComponentsToDelete;
	std::vector<std::pair<Entity, const ComponentType*>> m_ComponentsToRegister;

	float m_DeltaTime;

	std::mutex m_LockAddComponent, m_LockRemoveComponent, m_LockRemoveEntity;

private:
	static ECSCore* s_pInstance;
};

template<typename Comp>
inline Comp& ECSCore::AddComponent(Entity entity, const Comp& component)
{
	std::scoped_lock<std::mutex> lock(m_LockAddComponent);
	if (!m_ComponentStorage.HasType<Comp>()) {
		m_ComponentStorage.RegisterComponentType<Comp>();
	}

	/*	Create component immediately, but hold off on registering and publishing it until the end of the frame.
		This is to prevent concurrency issues. Publishing a component means pushing entity IDs to IDVectors,
		and there is no guarentee that no one is simultaneously reading from these IDVectors. */
	m_ComponentsToRegister.push_back({ entity, Comp::Type() });
	return m_ComponentStorage.AddComponent<Comp>(entity, component);
}

template<typename Comp>
inline Comp& ECSCore::GetComponent(Entity entity)
{
	return m_ComponentStorage.GetComponent<Comp>(entity);
}

template<typename Comp>
inline bool ECSCore::GetComponentIf(Entity entity, Comp** ppComp)
{
	return m_ComponentStorage.GetComponentIf<Comp>(entity, ppComp);
}

template<typename Comp>
inline const Comp& ECSCore::GetConstComponent(Entity entity) const
{
	return m_ComponentStorage.GetConstComponent<Comp>(entity);
}

template<typename Comp>
inline bool ECSCore::GetConstComponentIf(Entity entity, const Comp** ppComp) const
{
	return m_ComponentStorage.GetConstComponentIf<Comp>(entity, ppComp);
}

template<typename Comp>
inline ComponentArray<Comp>* ECSCore::GetComponentArray()
{
	return m_ComponentStorage.GetComponentArray<Comp>();
}

template<typename Comp>
inline const ComponentArray<Comp>* ECSCore::GetComponentArray() const
{
	return m_ComponentStorage.GetComponentArray<Comp>();
}

template<typename Comp>
inline void ECSCore::RemoveComponent(Entity entity)
{
	std::scoped_lock<std::mutex> lock(m_LockRemoveComponent);
	m_ComponentsToDelete.push_back({ entity, Comp::Type() });
}
