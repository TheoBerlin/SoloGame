#pragma once

#include <Engine/ECS/Entity.hpp>
#include <typeindex>
#include <vector>

class SystemHandler;

/*
    A component handler stores components and has functions for creating components and
    performing tasks on them (eg. perform transformations on transform components)
*/
class ComponentHandler
{
public:
    // Registers the component handler's type of components it handles
    ComponentHandler(std::vector<std::type_index> componentTypes, SystemHandler* systemHandler);

    // Deregisters component handler and deletes components
    ~ComponentHandler();

    // Returns which entities have a component of the given type
    virtual std::vector<Entity> getEntities(std::type_index componentType) = 0;

protected:
    // Tell the system handler a component has been created
    void registerComponent(std::type_index tid, Entity entityID);

    std::vector<std::type_index> handledTypes;

private:
    SystemHandler* systemHandler;
};
