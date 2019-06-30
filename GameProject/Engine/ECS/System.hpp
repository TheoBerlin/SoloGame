#pragma once

#include <Engine/ECS/Entity.hpp>
#include <Engine/Utils/IDVector.hpp>
#include <typeindex>
#include <vector>

class SystemSubscriber;
class ComponentHandler;
struct ComponentSubReq;

/*
    A system stores pointers to component storages and processes the components each frame
    in the update function
*/
class System
{
public:
    // Registers the system in the system handler
    System(SystemSubscriber* SystemSubscriber);

    // Deregisters system
    ~System();

    virtual void update(float dt) = 0;

    size_t ID;

protected:
    void subscribeToComponents(std::vector<ComponentSubReq>* subReqs);
    void unsubscribeFromComponents(std::vector<std::type_index> unsubTypes);
    ComponentHandler* getComponentHandler(std::type_index& componentType);

    std::vector<std::type_index> componentTypes;

private:
    SystemSubscriber* systemSubscriber;
};
