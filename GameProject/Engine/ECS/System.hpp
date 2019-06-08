#pragma once

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

    std::vector<std::type_index>& getComponentTypes();

    size_t ID;

protected:
    // Retrieves a pointer to a function given a component type
    ComponentHandler* getHandler(std::type_index componentType) const;

private:
    SystemHandler* systemHandler;
};
