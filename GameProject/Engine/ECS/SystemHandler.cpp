#include "SystemHandler.hpp"

#include <Engine/Utils/Logger.hpp>
#include <Engine/ECS/ComponentHandler.hpp>
#include <Engine/ECS/System.hpp>

SystemHandler::SystemHandler()
{}

SystemHandler::~SystemHandler()
{}

void SystemHandler::registerComponentHandler(std::type_index componentType, ComponentHandler* componentHandler)
{
    auto handlerItr = componentHandlers.find(componentType);

    if (handlerItr != componentHandlers.end()) {
        Logger::LOG_WARNING("Attempted to register already handled component type: %s", componentType.name());
        return;
    }

    componentHandlers[componentType] = componentHandler;
}

void SystemHandler::deregisterComponentHandler(std::type_index componentType)
{
    auto handlerItr = componentHandlers.find(componentType);

    if (handlerItr == componentHandlers.end()) {
        Logger::LOG_WARNING("Attempted to deregister a component handler for an unregistered component type: %s", componentType.name());
        return;
    }

    // Deregister all components so that systems don't try to process deleted components
    std::vector<Entity> entities = handlerItr->second->getEntities(componentType);

    for (size_t i = 0; i < entities.size(); i += 1) {
        this->removedComponent(entities[i], componentType);
    }

    componentHandlers.erase(handlerItr);
}

ComponentHandler* SystemHandler::getComponentHandler(std::type_index& componentType)
{
    return componentHandlers[componentType];
}

void SystemHandler::registerSystem(std::vector<std::type_index>& components, System* system)
{
    // Create component subscriptions
    for (size_t i = 0; i < components.size(); i += 1) {
        componentSubscriptions.insert({components[i], system});
    }

    size_t sysID = systemIdGen.genID();
    system->ID = sysID;

    systems.push_back(system, sysID);

    // Show the system all existing components it subscribed to
    for (size_t i = 0; i < components.size(); i += 1) {
        auto compHandlerItr = componentHandlers.find(components[i]);

        // A component handler might not exist for the subscribed component type...
        if (compHandlerItr == componentHandlers.end()) {
            continue;
        }

        std::vector<Entity> entities = compHandlerItr->second->getEntities(components[i]);

        for (size_t j = 0; j < entities.size(); j += 1) {
            system->newComponent(entities[j], components[i]);
        }
    }
}

void SystemHandler::deregisterSystem(System* system)
{
    systems.pop(system->ID);

    // Get system's component subscriptions
    const std::vector<std::type_index>& componentTypes = system->getComponentTypes();

    // Delete all component subscriptions for the system
    for (size_t i = 0; i < componentTypes.size(); i += 1) {
        for (auto subItr = componentSubscriptions.find(componentTypes[i]); subItr != componentSubscriptions.end(); subItr++) {
            if (subItr->second == system) {
                // Subscription found, delete it
                componentSubscriptions.erase(subItr);
                break;
            }
        }
    }
}

void SystemHandler::newComponent(Entity entityID, std::type_index componentType)
{
    for (auto subItr = componentSubscriptions.find(componentType); subItr != componentSubscriptions.end() && subItr->first == componentType; subItr++) {
        subItr->second->newComponent(entityID, componentType);
    }
}

void SystemHandler::removedComponent(Entity entityID, std::type_index componentType)
{
    for (auto subItr = componentSubscriptions.find(componentType); subItr != componentSubscriptions.end() && subItr->first == componentType; subItr++) {
        subItr->second->removedComponent(entityID, componentType);
    }
}
