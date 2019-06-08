#include "SystemHandler.hpp"

#include <Utils/Logger.hpp>
#include <Engine/ECS/System.hpp>

void SystemHandler::registerComponentHandler(std::type_index componentType, ComponentHandler* componentHandler)
{
    auto handlerItr = componentHandlers.find(componentType);

    if (handlerItr == componentHandlers.end()) {
        Logger::LOG_WARNING("Attempted to register already handled component type, index: %d\n", componentType.hash_code);
        return;
    }

    handlerItr->second = componentHandler;
}

void SystemHandler::deregisterComponentHandler(std::type_index componentType)
{
    auto handlerItr = componentHandlers.find(componentType);

    if (handlerItr == componentHandlers.end()) {
        Logger::LOG_WARNING("Attempted to deregister a component handler for an unregistered component type, index: %d\n", componentType.hash_code);
        return;
    }

    componentHandlers.erase(handlerItr);
}

ComponentHandler* SystemHandler::getComponentHandler(std::type_index componentType)
{
    return componentHandlers[componentType];
}

size_t SystemHandler::registerSystem(std::vector<std::type_index>& components, System* system)
{
    // Create a component subscription for each component type
    for (size_t i = 0; i < components.size(); i += 1) {
        componentSubscriptions.insert({components[i], system});
    }

    size_t sysID = systemIdGen.genID();
    system->ID = sysID;

    systems.push_back(system, sysID);
}

void SystemHandler::deregisterSystem(System* system)
{
    systems.pop(system->ID);

    std::vector<std::type_index>& componentTypes = system->getComponentTypes();

    // Delete all component subscriptions for the system
    for (size_t i = 0; i < componentTypes.size(); i += 1) {
        auto pairs = componentSubscriptions.find(componentTypes[i]);

        for (auto pair : pairs) {
            
        }
    }
}
