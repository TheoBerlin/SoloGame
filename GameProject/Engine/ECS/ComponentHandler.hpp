#pragma once

#include <Engine/ECS/Entity.hpp>
#include <functional>
#include <typeindex>
#include <vector>

class SystemSubscriber;

struct ComponentRegistration {
    std::type_index tid;
    std::function<bool(Entity)> componentQuery;
    // Vector of entities that have the given component type
    const std::vector<size_t>* entities;
};

/*
    A component handler stores components and has functions for creating components and
    performing tasks on them (eg. perform transformations on transform components)
*/
class ComponentHandler
{
public:
    ComponentHandler(std::vector<std::type_index> handledTypes, SystemSubscriber* systemSubscriber, std::type_index tid_handler);

    // Deregisters component handler and deletes components
    ~ComponentHandler();


    const std::vector<std::type_index>& getHandledTypes() const;
    std::type_index getHandlerType() const;

protected:
    /**
     * Registers the component handler's type of components it handles
     * @param componentQueries Functions for asking if an entity has a component of a certain type
     * @param entities Vectors containing the IDs of entities that have certain components
     */
    void registerHandler(std::vector<ComponentRegistration>* componentQueries);

    // Tell the system handler a component has been created
    void registerComponent(std::type_index tid, Entity entityID);

    std::vector<std::type_index> handledTypes;

private:
    std::type_index tid_handler;
    SystemSubscriber* systemSubscriber;
};
