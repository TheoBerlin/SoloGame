#include "SystemSubscriber.hpp"

#include <Engine/Utils/Logger.hpp>
#include <Engine/ECS/ComponentHandler.hpp>
#include <Engine/ECS/System.hpp>

SystemSubscriber::SystemSubscriber()
{}

SystemSubscriber::~SystemSubscriber()
{}

void SystemSubscriber::registerComponents(std::vector<ComponentRegistration>* componentRegs)
{
    for (const ComponentRegistration& componentReg : *componentRegs) {
        auto mapItr = this->componentContainers.find(componentReg.tid);

        if (mapItr != this->componentContainers.end()) {
            Logger::LOG_WARNING("Attempted to register an already handled component type: %s", componentReg.tid.name());
            continue;
        }

        this->componentContainers.insert({componentReg.tid, componentReg.componentContainer});
    }
}

void SystemSubscriber::deregisterComponents(ComponentHandler* handler)
{
    const std::vector<std::type_index>& componentTypes = handler->getHandledTypes();

    for (const std::type_index& componentType : componentTypes) {
        // Delete component query functions
        auto handlerItr = this->componentContainers.find(componentType);

        if (handlerItr == componentContainers.end()) {
            Logger::LOG_WARNING("Attempted to deregister a component handler for an unregistered component type: %s", componentType.name());
            continue;
        }

        componentContainers.erase(handlerItr);
    }

    auto handlerItr = componentHandlers.find(handler->getHandlerType());

    if (handlerItr == componentHandlers.end()) {
        Logger::LOG_WARNING("Attempted to deregister an unregistered component handler: %s", handler->getHandlerType().name());
        return;
    }

    componentHandlers.erase(handlerItr);
}

void SystemSubscriber::registerHandler(ComponentHandler* handler, std::type_index& handlerType)
{
    this->componentHandlers[handlerType] = handler;
}

ComponentHandler* SystemSubscriber::getComponentHandler(std::type_index& handlerType)
{
    auto itr = componentHandlers.find(handlerType);

    if (itr == componentHandlers.end()) {
        Logger::LOG_WARNING("Failed to retrieve component handler: %s", handlerType.name());
        return nullptr;
    }

    return itr->second;
}

void SystemSubscriber::registerSystem(SystemRegistration* sysReg)
{
    size_t sysID = systemIdGen.genID();

    // Create subscriptions from the subscription requests by finding the desired component containers
    std::vector<ComponentSubscriptions> subscriptions;
    subscriptions.reserve(sysReg->subReqs.size());

    for (const ComponentSubReq& subReq : sysReg->subReqs) {
        const std::vector<ComponentUpdateReg>& componentRegs = subReq.componentTypes;

        ComponentSubscriptions newSub;
        newSub.componentContainers.reserve(componentRegs.size());
        newSub.componentTypes.reserve(componentRegs.size());

        newSub.subscriber = subReq.subscriber;
        newSub.onEntityAdded = subReq.onEntityAdded;

        for (const ComponentUpdateReg& componentReg : componentRegs) {
            auto queryItr = componentContainers.find(componentReg.tid);

            if (queryItr == componentContainers.end()) {
                Logger::LOG_WARNING("Attempted to subscribe to unregistered component type: %s, %d", componentReg.tid.name(), componentReg.tid.hash_code());
                systemIdGen.popID(sysID);
                return;
            }

            newSub.componentContainers.push_back(queryItr->second);
            newSub.componentTypes.push_back(componentReg.tid);
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
    const std::vector<ComponentSubscriptions>& subs = subscriptionStorage.indexID(sysID);

    for (size_t i = 0; i < subs.size(); i += 1) {
        const std::vector<std::type_index>& componentTypes = subs[i].componentTypes;

        for (size_t j = 0; j < componentTypes.size(); j += 1) {
            componentSubscriptions.insert({componentTypes[j], {sysID, i}});
        }
    }

    // A subscription has been made, notify the system of all existing components it subscribed to
    for (ComponentSubscriptions& subscription : subscriptions) {
        // Fetch the entity vector of the first subscribed component type
        const std::vector<const IDContainer*>& componentContainers = subscription.componentContainers;
        const std::vector<Entity>& entityVec = componentContainers[0]->getIDs();

        // See which entities in the entity vector also have all the other component types. Register those entities in the system.
        for (Entity entity : entityVec) {
            bool registerEntity = true;

            for (size_t otherVecIdx = 1; otherVecIdx < componentContainers.size(); otherVecIdx += 1) {
                if (componentContainers[otherVecIdx]->hasElement(entity) == false) {
                    // The entity does not have this component type, do not register it
                    registerEntity = false;
                    break;
                }
            }

            if (registerEntity) {
                subscription.subscriber->push_back(entity, entity);

                if (subscription.onEntityAdded != nullptr) {
                    subscription.onEntityAdded(entity);
                }
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

    for (const ComponentSubscriptions& subscription : subscriptions) {
        const std::vector<std::type_index>& componentTypes = subscription.componentTypes;

        for (const std::type_index& componentType : componentTypes) {
            auto subBucketItr = componentSubscriptions.find(componentType);

            if (subBucketItr == componentSubscriptions.end()) {
                Logger::LOG_WARNING("Attempted to delete non-existent component subscription");
                // The other component subscriptions might exist, so don't return
                continue;
            }

            // Find the subscription and delete it
            while (subBucketItr != componentSubscriptions.end() && subBucketItr->first == componentType) {
                if (subBucketItr->second.systemID == system->ID) {
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

        // Query for every component's existence given the entity
        const std::vector<const IDContainer*>& componentContainers = sysSub.componentContainers;

        for (const IDContainer* componentContainer : componentContainers) {
            if (componentContainer->hasElement(entityID) == false) {
                triggerSub = false;
                break;
            }
        }

        if (triggerSub) {
            sysSub.subscriber->push_back(entityID, entityID);

            if (sysSub.onEntityAdded != nullptr) {
                sysSub.onEntityAdded(entityID);
            }
        }

        subBucketItr++;
    }

    if (registeredEntities.hasElement(entityID)) {
        registeredEntities.indexID(entityID).insert(componentType);
    } else {
        std::unordered_set<std::type_index> componentSet;
        componentSet.insert(componentType);
        registeredEntities.push_back(componentSet, entityID);
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

    registeredEntities.indexID(entityID).erase(componentType);
}

void SystemSubscriber::addDelayedDeletion(Entity entity)
{
    entitiesToDelete.push_back(entity);
}

void SystemSubscriber::performDeletions()
{
    for (Entity entity : entitiesToDelete) {
        const std::unordered_set<std::type_index>& componentTypes = registeredEntities.indexID(entity);

        // Tell every system that has the entity listed, that the entity has been deleted
        // Each call to removedComponent will remove the component type from the set
        while (!componentTypes.empty()) {
            this->removedComponent(entity, *componentTypes.begin());
        }

        registeredEntities.pop(entity);
    }

    entitiesToDelete.clear();
}

const std::vector<Entity>& SystemSubscriber::getEntitiesToDelete() const
{
    return entitiesToDelete;
}
