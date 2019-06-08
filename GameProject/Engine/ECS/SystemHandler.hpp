#pragma once

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
    ComponentHandler* getComponentHandler(std::type_index componentType);

    // Returns system ID
    size_t registerSystem(std::vector<std::type_index>& componentTypes, System* system);
    void deregisterSystem(size_t systemID);

private:
    IDVector<System*> systems;
    IDGenerator systemIdGen;
    std::multimap<std::type_index, System*> componentSubscriptions;

    std::map<std::type_index, ComponentHandler*> componentHandlers;
};
