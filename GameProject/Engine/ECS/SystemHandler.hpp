#pragma once

#include <Engine/ECS/Entity.hpp>
#include <Engine/Utils/IDGenerator.hpp>
#include <Engine/Utils/IDVector.hpp>

#include <vector>
#include <map>
#include <typeindex>

class System;
class ComponentHandler;

class SystemHandler
{
public:
    SystemHandler();
    ~SystemHandler();

    // Associates a component's type index with a component handler
    void registerComponentHandler(std::type_index componentType, ComponentHandler* componentHandler);
    void deregisterComponentHandler(std::type_index componentType);
    ComponentHandler* getComponentHandler(std::type_index& componentType);

    // Returns system ID
    void registerSystem(std::vector<std::type_index>& componentTypes, System* system);
    void deregisterSystem(System* system);

    // Notifies subscribed systems that a new component has been made
    void newComponent(Entity entityID, std::type_index componentType);
    // Notifies subscribed systems that a component has been deleted
    void removedComponent(Entity entityID, std::type_index componentType);

private:
    IDVector<System*> systems;
    IDGenerator systemIdGen;

    // Component subscriptions for systems, where the type index of a component type is the key
    std::multimap<std::type_index, System*> componentSubscriptions;

    // Map components to component handlers
    std::map<std::type_index, ComponentHandler*> componentHandlers;
};
