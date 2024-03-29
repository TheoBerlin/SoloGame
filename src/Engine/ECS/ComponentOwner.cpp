#include "Engine/ECS/ComponentOwner.hpp"

ComponentOwner::~ComponentOwner()
{
	ECSCore* pECS = ECSCore::GetInstance();
	if (pECS)
	{
		for (const ComponentType* pComponentType : m_OwnedComponentTypes)
		{
			pECS->UnsetComponentOwner(pComponentType);
		}
	}
}
