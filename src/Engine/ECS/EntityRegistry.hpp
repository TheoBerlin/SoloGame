#pragma once

#include <Engine/ECS/Entity.hpp>
#include <Engine/Utils/IDGenerator.hpp>
#include <Engine/Utils/IDVector.hpp>

#include <typeindex>
#include <stack>
#include <unordered_set>

// Map Entities to the set of component types they are registered to
typedef IDDVector<std::unordered_set<std::type_index>> EntityRegistryPage;

class EntityRegistry
{
public:
    EntityRegistry();
    ~EntityRegistry();

    void registerComponentType(Entity entity, std::type_index componentType);
    void deregisterComponentType(Entity entity, std::type_index componentType);

    // Queries whether or not the entity has all the specified types
    bool entityHasTypes(Entity entity, const std::vector<std::type_index>& queryTypes) const;

    Entity createEntity();
    void deregisterEntity(Entity entity);

    void addPage();
    void removePage();
    const EntityRegistryPage& getTopRegistryPage() const { return m_EntityPages.top(); }

private:
    std::stack<EntityRegistryPage> m_EntityPages;
    IDGenerator m_EntityIDGen;
};
