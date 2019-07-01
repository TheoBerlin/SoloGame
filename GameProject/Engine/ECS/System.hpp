#pragma once

#include <Engine/ECS/Entity.hpp>
#include <Engine/Utils/IDVector.hpp>
#include <typeindex>
#include <vector>

enum ComponentPermissions {
    R,
    W,
    RW
};

struct ComponentUpdateReg {
    ComponentPermissions permissions;
    std::type_index tid;
};

// A request for a system to subscribe to one or more component types
struct ComponentSubReq {
    std::vector<ComponentUpdateReg> componentTypes;
    IDVector<Entity>* subscriber;
};

class System;

struct SystemRegistration {
    std::vector<ComponentSubReq> subReqs;
    System* system;
};

struct ECSInterface;
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
    System(ECSInterface* ecs);

    // Deregisters system
    ~System();

    virtual void update(float dt) = 0;

    size_t ID;

protected:
    void subscribeToComponents(SystemRegistration* sysReg);
    void registerUpdate(SystemRegistration* sysReg);

    void unsubscribeFromComponents(std::vector<std::type_index> unsubTypes);
    ComponentHandler* getComponentHandler(std::type_index& componentType);

    std::vector<std::type_index> componentTypes;

private:
    ECSInterface* ecs;
};
