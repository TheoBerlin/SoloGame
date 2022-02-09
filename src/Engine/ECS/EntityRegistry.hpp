#pragma once

#include "Engine/ECS/Entity.hpp"
#include "Engine/Utils/IDGenerator.hpp"
#include "Engine/Utils/IDVector.hpp"

#include <mutex>
#include <stack>
#include <typeindex>
#include <unordered_set>

class ComponentType;

// Map Entities to the set of component types they are registered to
typedef IDDVector<std::unordered_set<const ComponentType*>> EntityRegistryPage;

class EntityRegistry
{
public:
    EntityRegistry();
    ~EntityRegistry() = default;

    void RegisterComponentType(Entity entity, const ComponentType* pComponentType);
    void DeregisterComponentType(Entity entity, const ComponentType* pComponentType);

    // EntityHasAllTypes returns true if the entity has all of the allowed types and none of the disallowed types
    bool EntityHasAllowedTypes(Entity entity, const std::vector<const ComponentType*>& allowedTypes, const std::vector<const ComponentType*>& disallowedTypes) const;
    // EntityHasAllTypes returns true if the entity has all of the specified types
    bool EntityHasAllTypes(Entity entity, const std::vector<const ComponentType*>& types) const;
    // EntityHasAnyOfTypes returns true if the entity has at least one of the specified types
    bool EntityHasAnyOfTypes(Entity entity, const std::vector<const ComponentType*>& types) const;

    Entity CreateEntity();
    void DeregisterEntity(Entity entity);

    void AddPage();
    void RemovePage();
    const EntityRegistryPage& GetTopRegistryPage() const { return m_EntityPages.top(); }

private:
    std::stack<EntityRegistryPage> m_EntityPages;
    IDGenerator m_EntityIDGen;
    mutable std::mutex m_Lock;
};
