#pragma once

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

protected:
    SystemHandler* systemHandler;
    std::vector<std::type_index> handledTypes;
};
