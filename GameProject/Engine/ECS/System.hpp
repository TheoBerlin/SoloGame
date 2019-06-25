#pragma once

#include <Engine/ECS/Entity.hpp>
#include <Engine/Utils/IDVector.hpp>
#include <typeindex>
#include <vector>

class SystemHandler;
class ComponentHandler;

/*
    A system stores pointers to component storages and processes the components each frame
    in the update function
*/
class System
{
public:
    // Registers the system in the system handler
    System(SystemHandler* systemHandler);

    // Deregisters system
    ~System();

    virtual void update(float dt) = 0;

    // Notify the system that a component of a subscribed type has been made
    virtual void newComponent(Entity entityID, std::type_index componentType) = 0;
    // Notify the system that a component of a subscribed type has been removed
    virtual void removedComponent(Entity entityID, std::type_index componentType) = 0;

    const std::vector<std::type_index>& getComponentTypes() const;

    size_t ID;

protected:
    void subscribeToComponents();
    ComponentHandler* getComponentHandler(std::type_index& componentType);

    std::vector<std::type_index> componentTypes;

    // Entities to handle in update function
    IDVector<Entity> entityIDs;

private:
    SystemHandler* systemHandler;
};
