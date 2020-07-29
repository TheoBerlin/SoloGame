#pragma once

#include <Engine/ECS/Entity.hpp>
#include <functional>
#include <typeindex>
#include <vector>

class ECSCore;
class IDContainer;

struct ComponentRegistration {
    std::type_index tid;
    IDContainer* pComponentContainer;
    std::function<void(Entity)> m_ComponentDestructor;
};

class ComponentHandler;

struct ComponentHandlerRegistration {
    ComponentHandler* pComponentHandler;
    std::vector<ComponentRegistration> ComponentRegistrations;
    std::vector<std::type_index> HandlerDependencies;
};

/*
    A component handler stores components and has functions for creating components and
    performing tasks on them (eg. perform transformations on transform components)
*/
class ComponentHandler
{
public:
    ComponentHandler(ECSCore* pECS, std::type_index tid_handler);
    // Deregisters component handler and deletes components
    virtual ~ComponentHandler();

    virtual bool initHandler() = 0;

    const std::vector<std::type_index>& getHandledTypes() const;
    std::type_index getHandlerType() const;

protected:
    /**
     * Registers the component handler's type of components it handles
     * @param componentQueries Functions for asking if an entity has a component of a certain type
     */
    void registerHandler(const ComponentHandlerRegistration& handlerRegistration);

    // Tell the system subscriber a component has been created
    void registerComponent(Entity entity, std::type_index componentType);

protected:
    ECSCore* m_pECS;
    std::vector<std::type_index> m_HandledTypes;

private:
    std::type_index m_TID;
};
