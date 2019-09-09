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
    for (size_t i = 0; i < componentRegs->size(); i += 1) {
        auto mapItr = this->componentContainers.find((*componentRegs)[i].tid);

        if (mapItr != this->componentContainers.end()) {
            Logger::LOG_WARNING("Attempted to register an already handled component type: %s", (*componentRegs)[i].tid.name());
            continue;
        }

        this->componentContainers.insert({componentRegs->at(i).tid, componentRegs->at(i).componentContainer});
    }
}

void SystemSubscriber::deregisterComponents(ComponentHandler* handler)
{
    const std::vector<std::type_index>& componentTypes = handler->getHandledTypes();

    for (size_t i = 0; i < componentTypes.size(); i += 1) {
        // Delete component query functions
        auto handlerItr = this->componentContainers.find(componentTypes[i]);

        if (handlerItr == componentContainers.end()) {
            Logger::LOG_WARNING("Attempted to deregister a component handler for an unregistered component type: %s", componentTypes[i].name());
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

    for (size_t i = 0; i < sysReg->subReqs.size(); i += 1) {
        const std::vector<ComponentUpdateReg>& componentRegs = sysReg->subReqs[i].componentTypes;

        ComponentSubscriptions newSub;
        newSub.componentContainers.reserve(componentRegs.size());
        newSub.componentTypes.reserve(componentRegs.size());

        newSub.subscriber = sysReg->subReqs[i].subscriber;

        for (size_t j = 0; j < componentRegs.size(); j += 1) {
            auto queryItr = componentContainers.find(componentRegs[j].tid);

            if (queryItr == componentContainers.end()) {
                Logger::LOG_WARNING("Attempted to subscribe to unregistered component type: %s, %d", componentRegs[j].tid.name(), componentRegs[j].tid.hash_code());
                systemIdGen.popID(sysID);
                return;
            }

            newSub.componentContainers.push_back(queryItr->second);
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
    const std::vector<ComponentSubscriptions>& subs = subscriptionStorage.indexID(sysID);

    for (size_t i = 0; i < subs.size(); i += 1) {
        const std::vector<std::type_index>& componentTypes = subs[i].componentTypes;

        for (size_t j = 0; j < componentTypes.size(); j += 1) {
            componentSubscriptions.insert({componentTypes[j], {sysID, i}});
        }
    }

    // A subscription has been made, notify the system of all existing components it subscribed to
    for (size_t i = 0; i < subscriptions.size(); i += 1) {
        // Fetch the entity vector of the first subscribed component type
        const std::vector<const IDContainer*>& componentContainers = subscriptions[i].componentContainers;
        const std::vector<Entity>& entityVec = componentContainers[0]->getIDs();

        // See which entities in the entity vector also have all the other component types. Register those entities in the system.
        for (size_t entityIdx = 0; i < entityVec.size(); i += 1) {
            bool registerEntity = true;

            Entity entity = entityVec.at(entityIdx);

            for (size_t otherVecIdx = 1; otherVecIdx < componentContainers.size(); otherVecIdx += 1) {
                if (componentContainers[otherVecIdx]->hasElement(entity) == false) {
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
        const std::vector<std::type_index>& componentTypes = subscriptions[i].componentTypes;

        for (size_t j = 0; j < componentTypes.size(); j += 1) {
            auto subBucketItr = componentSubscriptions.find(componentTypes[j]);

            if (subBucketItr == componentSubscriptions.end()) {
                Logger::LOG_WARNING("Attempted to delete non-existent component subscription");
                // The other component subscriptions might exist, so don't return
                continue;
            }

            // Find the subscription and delete it
            while (subBucketItr != componentSubscriptions.end() && subBucketItr->first == componentTypes[j]) {
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

        for (size_t i = 0; i < componentContainers.size(); i += 1) {
            if (componentContainers[i]->hasElement(entityID) == false) {
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
