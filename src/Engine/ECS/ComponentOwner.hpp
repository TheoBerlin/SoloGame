#pragma once

#include "Engine/ECS/Component.hpp"
#include "Engine/ECS/ECSCore.hpp"

class ComponentOwner
{
public:
	ComponentOwner() = default;
	~ComponentOwner();

	template <typename Comp>
	void SetComponentOwner(const ComponentOwnership<Comp>& componentOwnership);

private:
	std::vector<const ComponentType*> m_OwnedComponentTypes;
};

template <typename Comp>
inline void ComponentOwner::SetComponentOwner(const ComponentOwnership<Comp>& componentOwnership)
{
	ECSCore::GetInstance()->SetComponentOwner(componentOwnership);
	m_OwnedComponentTypes.push_back(Comp::Type());
}
