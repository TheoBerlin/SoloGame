#include "Engine/ECS/ComponentStorage.hpp"

ComponentStorage::~ComponentStorage()
{
	for (IComponentArray* pCompArr : m_ComponentArrays) {
		delete pCompArr;
	}
}

void ComponentStorage::UnsetComponentOwner(const ComponentType* pComponentType)
{
	IComponentArray* pCompArray = GetComponentArray(pComponentType);
	pCompArray->UnsetComponentOwner();
}

bool ComponentStorage::DeleteComponent(Entity entity, const ComponentType* pComponentType)
{
	IComponentArray* pComponentArray = GetComponentArray(pComponentType);
	if (pComponentArray) {
		pComponentArray->Remove(entity);
		return true;
	} else {
		return false;
	}
}

uint32_t ComponentStorage::SerializeComponent(Entity entity, const ComponentType* pComponentType, uint8_t* pBuffer, uint32_t bufferSize) const
{
	const IComponentArray* pComponentArray = GetComponentArray(pComponentType);
	return pComponentArray->SerializeComponent(entity, pBuffer, bufferSize);
}

bool ComponentStorage::DeserializeComponent(Entity entity, const ComponentType* pComponentType, uint32_t componentDataSize, const uint8_t* pBuffer, bool& entityHadComponent)
{
	IComponentArray* pComponentArray = GetComponentArray(pComponentType);
	return pComponentArray->DeserializeComponent(entity, pBuffer, componentDataSize, entityHadComponent);
}

IComponentArray* ComponentStorage::GetComponentArray(const ComponentType* pComponentType)
{
	auto arrayItr = m_CompTypeToArrayMap.find(pComponentType);
	return arrayItr == m_CompTypeToArrayMap.end() ? nullptr : m_ComponentArrays[arrayItr->second];
}

const IComponentArray* ComponentStorage::GetComponentArray(const ComponentType* pComponentType) const
{
	auto arrayItr = m_CompTypeToArrayMap.find(pComponentType);
	return arrayItr == m_CompTypeToArrayMap.end() ? nullptr : m_ComponentArrays[arrayItr->second];
}

void ComponentStorage::ResetDirtyFlags()
{
	for (IComponentArray* pArray : m_ComponentArraysWithDirtyFlags) {
		pArray->ResetDirtyFlags();
	}
}

const ComponentType* ComponentStorage::GetComponentType(uint32_t componentTypeHash) const
{
	auto componentTypeItr = m_TypeHashToCompTypeMap.find(componentTypeHash);
	return componentTypeItr != m_TypeHashToCompTypeMap.end() ? componentTypeItr->second : nullptr;
}
