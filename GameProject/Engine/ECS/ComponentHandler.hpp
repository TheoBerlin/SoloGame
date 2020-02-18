#pragma once

#include <Engine/ECS/Entity.hpp>
#include <functional>
#include <typeindex>
#include <vector>

class ECSCore;
class IDContainer;

struct ComponentRegistration {
    std::type_index tid;
    IDContainer* componentContainer;
    std::function<void(Entity)> m_ComponentDestructor;
};

/*
    A component handler stores components and has functions for creating components and
    performing tasks on them (eg. perform transformations on transform components)
*/
class ComponentHandler
{
public:
    ComponentHandler(std::vector<std::type_index> handledTypes, ECSCore* pECS, std::type_index tid_handler);

    // Deregisters component handler and deletes components
    ~ComponentHandler();

    const std::vector<std::type_index>& getHandledTypes() const;
    std::type_index getHandlerType() const;

protected:
    /**
     * Registers the component handler's type of components it handles
     * @param componentQueries Functions for asking if an entity has a component of a certain type
     */
    void registerHandler(std::vector<ComponentRegistration>* componentQueries);

    // Tell the system handler a component has been created
    void registerComponent(Entity entity, std::type_index componentType);

    std::vector<std::type_index> m_HandledTypes;

private:
    std::type_index m_TID;
    ECSCore* m_pECS;
};
