#pragma once

#include "Engine/ECS/Component.hpp"
#include "Engine/ECS/Entity.hpp"

#include <type_traits>

class ComponentStorage;

#pragma pack(push, 1)
	struct ComponentSerializationHeader {
		uint32_t TotalSerializationSize; // Size of header + component data
		uint32_t TypeHash;
	};
#pragma pack(pop)

class IComponentArray
{
public:
	virtual ~IComponentArray() = default;

	virtual void UnsetComponentOwner() = 0;

	virtual const std::vector<uint32_t>& GetIDs() const = 0;

	virtual uint32_t SerializeComponent(Entity entity, uint8_t* pBuffer, uint32_t bufferSize) const = 0;
	// DeserializeComponent adds a component if it does not already exist, otherwise the existing component is updated
	virtual bool DeserializeComponent(Entity entity, const uint8_t* pBuffer, uint32_t serializationSize, bool& entityHadComponent) = 0;

	virtual bool HasComponent(Entity entity) const = 0;
	virtual void ResetDirtyFlags() = 0;

	virtual void* GetRawData(Entity entity) = 0;

protected:
	// Systems or other external users should not be able to perform immediate deletions
	friend ComponentStorage;
	virtual void Remove(Entity entity) = 0;
};

template<typename Comp>
class ComponentArray : public IComponentArray
{
public:
	ComponentArray() = default;
	~ComponentArray() override final;

	void SetComponentOwner(const ComponentOwnership<Comp>& componentOwnership) { m_ComponentOwnership = componentOwnership; }
	void UnsetComponentOwner() override final { m_ComponentOwnership = {}; }

	Comp& Insert(Entity entity, const Comp& comp);

	// Fills comp with component data, sets dirty flag if one exists and returns whether the component exists
	bool GetIf(Entity entity, Comp** ppComp);
	// Fills comp with component data and returns whether the component exists
	bool GetConstIf(Entity entity, const Comp** ppComp) const;
	Comp& GetData(Entity entity);
	const Comp& GetConstData(Entity entity) const;

	void* GetRawData(Entity entity) override final;

	const std::vector<uint32_t>& GetIDs() const override final { return m_IDs; }

	uint32_t SerializeComponent(Entity entity, uint8_t* pBuffer, uint32_t bufferSize) const override final { return SerializeComponent(GetConstData(entity), pBuffer, bufferSize); }
	uint32_t SerializeComponent(const Comp& component, uint8_t* pBuffer, uint32_t bufferSize) const;
	bool DeserializeComponent(Entity entity, const uint8_t* pBuffer, uint32_t serializationSize, bool& entityHadComponent);

	bool HasComponent(Entity entity) const override final { return m_EntityToIndex.find(entity) != m_EntityToIndex.end(); }
	void ResetDirtyFlags() override final;

protected:
	void Remove(Entity entity) override final;

private:
	std::vector<Comp> m_Data;
	std::vector<uint32_t> m_IDs;
	std::unordered_map<Entity, uint32_t> m_EntityToIndex;

	ComponentOwnership<Comp> m_ComponentOwnership;
};

template<typename Comp>
inline ComponentArray<Comp>::~ComponentArray()
{
	if (m_ComponentOwnership.Destructor) {
		for (uint32_t componentIdx = 0; componentIdx < m_Data.size(); componentIdx++) {
			m_ComponentOwnership.Destructor(m_Data[componentIdx], m_IDs[componentIdx]);
		}
	}
}

template<typename Comp>
inline Comp& ComponentArray<Comp>::Insert(Entity entity, const Comp& comp)
{
	auto indexItr = m_EntityToIndex.find(entity);

	// Get new index and add the component to that position.
	m_EntityToIndex[entity] = (uint32_t)m_Data.size();
	m_IDs.push_back(entity);
	m_Data.push_back(comp);

	Comp& storedComp = m_Data.back();
	if (m_ComponentOwnership.Constructor) {
		m_ComponentOwnership.Constructor(storedComp, entity);
	}

	return storedComp;
}

template<typename Comp>
bool ComponentArray<Comp>::GetIf(Entity entity, Comp** ppComp)
{
	auto indexItr = m_EntityToIndex.find(entity);
	if (indexItr == m_EntityToIndex.end()) {
		return false;
	}

	*ppComp = &m_Data[indexItr->second];

	if constexpr (Comp::HasDirtyFlag()) {
		(*ppComp)->Dirty = true;
	}

	return true;
}

template<typename Comp>
bool ComponentArray<Comp>::GetConstIf(Entity entity, const Comp** ppComp) const
{
	auto indexItr = m_EntityToIndex.find(entity);
	if (indexItr == m_EntityToIndex.end()) {
		return false;
	}

	*ppComp = &m_Data[indexItr->second];

	return true;
}

template<typename Comp>
inline void* ComponentArray<Comp>::GetRawData(Entity entity)
{
	auto indexItr = m_EntityToIndex.find(entity);
	return &m_Data[indexItr->second];
}

template<typename Comp>
inline Comp& ComponentArray<Comp>::GetData(Entity entity)
{
	auto indexItr = m_EntityToIndex.find(entity);

	Comp& component = m_Data[indexItr->second];

	if constexpr (Comp::HasDirtyFlag()) {
		component.Dirty = true;
	}

	return component;
}

template<typename Comp>
inline const Comp& ComponentArray<Comp>::GetConstData(Entity entity) const
{
	auto indexItr = m_EntityToIndex.find(entity);
	return m_Data[indexItr->second];
}

template<typename Comp>
inline void ComponentArray<Comp>::Remove(Entity entity)
{
	auto indexItr = m_EntityToIndex.find(entity);

	uint32_t currentIndex = indexItr->second;

	if (m_ComponentOwnership.Destructor) {
		m_ComponentOwnership.Destructor(m_Data[currentIndex], entity);
	}

	// Swap the removed component with the last component.
	m_Data[currentIndex] = m_Data.back();
	m_IDs[currentIndex] = m_IDs.back();

	// Update entity-index maps.
	m_EntityToIndex[m_IDs.back()] = currentIndex;

	m_Data.pop_back();
	m_IDs.pop_back();

	// Remove the deleted component's entry.
	m_EntityToIndex.erase(indexItr);
}

template <typename Comp>
inline uint32_t ComponentArray<Comp>::SerializeComponent(const Comp& component, uint8_t* pBuffer, uint32_t bufferSize) const
{
	/*	ComponentSerializationHeader is written to the beginning of the buffer. This is done last, when the size of
		the serialization is known. */
	uint8_t* pHeaderPosition = pBuffer;
	constexpr const uint32_t headerSize = sizeof(ComponentSerializationHeader);
	const bool hasRoomForHeader = bufferSize >= headerSize;
	if (hasRoomForHeader) {
		pBuffer		+= headerSize;
		bufferSize	-= headerSize;
	}

	uint32_t requiredTotalSize = headerSize;

	// Use a component owner's serialize function, or memcpy the component directly
	if (m_ComponentOwnership.Serialize) {
		requiredTotalSize += m_ComponentOwnership.Serialize(component, pBuffer, bufferSize);
	}
	else if constexpr (std::is_trivially_copyable<Comp>::value) {
		// The if-statements have to be nested to avoid the compiler warning: 'use constexpr on if-statement'
		if (bufferSize >= sizeof(Comp)) {
			constexpr const uint32_t componentSize = sizeof(Comp);
			memcpy(pBuffer, &component, componentSize);
			requiredTotalSize += componentSize;
		}
	}

	// Finalize the serialization by writing the header
	if (hasRoomForHeader) {
		const ComponentSerializationHeader header = {
			.TotalSerializationSize	= requiredTotalSize,
			.TypeHash				= (uint32_t)Comp::Type()->Hash()
		};

		memcpy(pHeaderPosition, &header, headerSize);
	}

	return requiredTotalSize;
}

template <typename Comp>
inline bool ComponentArray<Comp>::DeserializeComponent(Entity entity, const uint8_t* pBuffer, uint32_t serializationSize, bool& entityHadComponent)
{
	Comp component = {};
	Comp* pComponent = &component;
	entityHadComponent = false;
	if (HasComponent(entity)) {
		entityHadComponent = true;
		pComponent = &GetData(entity);
	}

	if (m_ComponentOwnership.Deserialize) {
		return m_ComponentOwnership.Deserialize(*pComponent, serializationSize, pBuffer);
	}

	memcpy(pComponent, pBuffer, serializationSize);
	if (!entityHadComponent) {
		Insert(entity, *pComponent);
	}

	return true;
}

template<typename Comp>
inline void ComponentArray<Comp>::ResetDirtyFlags()
{
	if constexpr (Comp::HasDirtyFlag()) {
		for (Comp& component : m_Data) {
			component.Dirty = false;
		}
	}
}
