#include "SystemSubscriber.hpp"

#include <Engine/Utils/Logger.hpp>
#include <Engine/ECS/ComponentHandler.hpp>
#include <Engine/ECS/System.hpp>

SystemSubscriber::SystemSubscriber()
{}

SystemSubscriber::~SystemSubscriber()
{}

void SystemSubscriber::registerComponents(ComponentHandler* handler, std::vector<ComponentRegistration>* componentRegs)
{
    for (size_t i = 0; i < componentRegs->size(); i += 1) {
        auto mapItr = this->componentResources.find((*componentRegs)[i].tid);

        if (mapItr != this->componentResources.end()) {
            Logger::LOG_WARNING("Attempted to register an already handled component type: %s", (*componentRegs)[i].tid.name());
            continue;
        }

        ComponentResources componentResources = {
            (*componentRegs)[i].componentQuery,
            (*componentRegs)[i].entities
        };

        this->componentResources.insert({(*componentRegs)[i].tid, componentResources});
    }

    this->componentHandlers[handler->getHandlerType()] = handler;
}

void SystemSubscriber::deregisterComponents(ComponentHandler* handler)
{
    const std::vector<std::type_index>& componentTypes = handler->getHandledTypes();

    for (size_t i = 0; i < componentTypes.size(); i += 1) {
        // Delete component query functions
        auto handlerItr = this->componentResources.find(componentTypes[i]);

        if (handlerItr == componentResources.end()) {
            Logger::LOG_WARNING("Attempted to deregister a component handler for an unregistered component type: %s", componentTypes[i].name());
            continue;
        }

        componentResources.erase(handlerItr);
    }

    auto handlerItr = componentHandlers.find(handler->getHandlerType());

    if (handlerItr == componentHandlers.end()) {
        Logger::LOG_WARNING("Attempted to deregister an unregistered component handler: %s", handler->getHandlerType().name());
        return;
    }

    componentHandlers.erase(handlerItr);
}

ComponentHandler* SystemSubscriber::getComponentHandler(std::type_index& componentType)
{
    return componentHandlers[componentType];
}

void SystemSubscriber::registerSystem(SystemRegistration* sysReg)
{
    size_t sysID = systemIdGen.genID();

    // Convert subscription requests to subscriptions by finding the components' resources
    std::vector<ComponentSubscriptions> subscriptions;
    subscriptions.reserve(sysReg->subReqs.size());

    for (size_t i = 0; i < sysReg->subReqs.size(); i += 1) {
        const std::vector<ComponentUpdateReg>& componentRegs = sysReg->subReqs[i].componentTypes;

        ComponentSubscriptions newSub;
        newSub.entityHasComponent.reserve(componentRegs.size());
        newSub.componentTypes.reserve(componentRegs.size());

        newSub.subscriber = sysReg->subReqs[i].subscriber;

        for (size_t j = 0; j < componentRegs.size(); j += 1) {
            auto queryItr = componentResources.find(componentRegs[j].tid);

            if (queryItr == componentResources.end()) {
                Logger::LOG_WARNING("Attempted to subscribe to unregistered component type: %s", componentRegs[i].tid.name());
                systemIdGen.popID(sysID);
                return;
            }

            newSub.entityHasComponent.push_back(&queryItr->second.componentQuery);
            newSub.componentTypes.push_back(componentRegs[j].tid);

        }

        subscriptions.push_back(newSub);
    }

    if (subscriptions.size() == 0) {
        Logger::LOG_WARNING("No subscriptions were able to be made during a system registration");
        return;
    }

    sysReg->system->ID = sysID;
    this->subscriptionStorage.push_back(subscriptions, sysID);

    // Map each component type to its subscriptions
    std::vector<ComponentSubscriptions>& subs = subscriptionStorage.indexID(sysID);

    for (size_t i = 0; i < subs.size(); i += 1) {
        std::vector<std::type_index>& componentTypes = subs[i].componentTypes;

        for (size_t j = 0; j < componentTypes.size(); j += 1) {
            componentSubscriptions.insert({componentTypes[j], {sysID, i}});
        }
    }

    // A subscription has been made, notify the system of all existing components it subscribed to
    for (size_t i = 0; i < subscriptions.size(); i += 1) {
        /*
            1. Fetch the entity ID vector of the first subscribed component type
            2. Fetch the entity existence query functions for all the other component types in the subscriptions.
            3. See which entities in the ID vector also have all the other component types. Register those entities in the system.
        */

        // 1.
        std::vector<std::type_index>& componentTypes = subscriptions[i].componentTypes;

        ComponentResources& componentResources = this->componentResources[componentTypes[0]];

        const std::vector<Entity>* entityVec = componentResources.entities;

        // 2.
        std::vector<std::function<bool(Entity)>*>& queryFuncs = subscriptions[i].entityHasComponent;

        // 3.
        for (size_t entityIdx = 0; entityIdx < entityVec->size(); entityIdx += 1) {
            bool registerEntity = true;

            Entity entity = entityVec->at(entityIdx);

            for (size_t queryIdx = 1; queryIdx < queryFuncs.size(); queryIdx += 1) {
                if ((*queryFuncs[queryIdx])(entity) == false) {
                    // The entity does not have this component type, do not register it
                    registerEntity = false;
                    break;
                }
            }

            if (registerEntity) {
                subscriptions[i].subscriber->push_back(entity, entity);
            }
        }
    }
}

void SystemSubscriber::deregisterSystem(System* system, std::vector<std::type_index>& componentTypes)
{
    if (subscriptionStorage.hasElement(system->ID) == false) {
        Logger::LOG_WARNING("Attempted to deregistered an unregistered system, ID: %d", system->ID);
        return;
    }

    // Use the subscriptions to find and delete component subscriptions
    std::vector<ComponentSubscriptions>& subscriptions = subscriptionStorage.indexID(system->ID);

    for (size_t i = 0; i < subscriptions.size(); i += 1) {
        std::vector<std::type_index>& componentTypes = subscriptions[i].componentTypes;

        for (size_t j = 0; j < componentTypes.size(); j += 1) {
            auto subBucketItr = componentSubscriptions.find(componentTypes[j]);

            if (subBucketItr == componentSubscriptions.end()) {
                Logger::LOG_WARNING("Attempted to delete non-existent component subscription");
                // The other component subscriptions might exist, so don't return
                continue;
            }

            // Iterate through the unordered_map bucket
            while (subBucketItr != componentSubscriptions.end() && subBucketItr->first == componentTypes[j]) {
                if (subBucketItr->second.systemID == system->ID) {
                    // Found the subscription, erase it and move on to the next one
                    componentSubscriptions.erase(subBucketItr);
                    break;
                }

                subBucketItr++;
            }
        }
    }

    // All component->subscription mappings have been deleted
    subscriptionStorage.pop(system->ID);

    // Recycle system iD
    systemIdGen.popID(system->ID);
}

void SystemSubscriber::newComponent(Entity entityID, std::type_index componentType)
{
    // Get all subscriptions for the component type by iterating through the unordered_map bucket
    auto subBucketItr = componentSubscriptions.find(componentType);

    while (subBucketItr != componentSubscriptions.end() && subBucketItr->first == componentType) {
        // Use indices stored in the component type -> component storage mapping to get the component subscription
        ComponentSubscriptions& sysSub = subscriptionStorage.indexID(subBucketItr->second.systemID)[subBucketItr->second.subIdx];

        if (sysSub.subscriber->hasElement(entityID)) {
            subBucketItr++;
            continue;
        }

        // Whether or not the entity has all the subscribed component types
        bool triggerSub = true;

        // Query for every componen's existence given the entity
        for (size_t i = 0; i < sysSub.entityHasComponent.size(); i += 1) {
            if ((*sysSub.entityHasComponent[i])(entityID) == false) {
                triggerSub = false;
                break;
            }
        }

        if (triggerSub) {
            sysSub.subscriber->push_back(entityID, entityID);
        }

        subBucketItr++;
    }
}

void SystemSubscriber::removedComponent(Entity entityID, std::type_index componentType)
{
    // Get all subscriptions for the component type by iterating through the unordered_map bucket
    auto subBucketItr = componentSubscriptions.find(componentType);

    while (subBucketItr != componentSubscriptions.end() && subBucketItr->first == componentType) {
        // Use indices stored in the component type -> component storage mapping to get the component subscription
        ComponentSubscriptions& sysSub = subscriptionStorage.indexID(subBucketItr->second.systemID)[subBucketItr->second.subIdx];

        if (sysSub.subscriber->hasElement(entityID) == false) {
            subBucketItr++;
            continue;
        }

        sysSub.subscriber->pop(entityID);

        subBucketItr++;
    }
}
