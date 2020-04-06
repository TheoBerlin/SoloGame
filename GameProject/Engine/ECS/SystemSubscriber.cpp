#include "SystemSubscriber.hpp"

#include <Engine/ECS/ComponentHandler.hpp>
#include <Engine/ECS/System.hpp>
#include <Engine/Utils/Logger.hpp>

SystemSubscriber::SystemSubscriber(EntityRegistry* pEntityRegistry)
    :m_pEntityRegistry(pEntityRegistry)
{}

SystemSubscriber::~SystemSubscriber()
{}


void SystemSubscriber::registerComponentHandler(const ComponentHandlerRegistration& componentHandlerRegistration)
{
    ComponentHandler* pComponentHandler = componentHandlerRegistration.pComponentHandler;
    std::type_index tidHandler = pComponentHandler->getHandlerType();

    this->componentHandlers[tidHandler] = pComponentHandler;

    // Register component containers
    for (const ComponentRegistration& componentReg : componentHandlerRegistration.ComponentRegistrations) {
        auto mapItr = this->m_ComponentStorage.find(componentReg.tid);

        if (mapItr != this->m_ComponentStorage.end()) {
            Logger::LOG_WARNING("Attempted to register an already handled component type: %s", componentReg.tid.name());
            continue;
        }

        this->m_ComponentStorage.insert({componentReg.tid, {componentReg.pComponentContainer, componentReg.m_ComponentDestructor}});
    }
}

void SystemSubscriber::deregisterComponentHandler(ComponentHandler* handler)
{
    const std::vector<std::type_index>& componentTypes = handler->getHandledTypes();

    for (const std::type_index& componentType : componentTypes) {
        // Delete component query functions
        auto handlerItr = m_ComponentStorage.find(componentType);

        if (handlerItr == m_ComponentStorage.end()) {
            Logger::LOG_WARNING("Attempted to deregister a component handler for an unregistered component type: %s", componentType.name());
            continue;
        }

        const std::vector<Entity>& entities = handlerItr->second.m_pContainer->getIDs();
        for (Entity entity : entities) {
            removedComponent(entity, componentType);
        }

        m_ComponentStorage.erase(handlerItr);
    }

    auto handlerItr = componentHandlers.find(handler->getHandlerType());

    if (handlerItr == componentHandlers.end()) {
        Logger::LOG_WARNING("Attempted to deregister an unregistered component handler: %s", handler->getHandlerType().name());
        return;
    }

    componentHandlers.erase(handlerItr);
}

ComponentHandler* SystemSubscriber::getComponentHandler(const std::type_index& handlerType)
{
    auto itr = componentHandlers.find(handlerType);

    if (itr == componentHandlers.end()) {
        Logger::LOG_WARNING("Failed to retrieve component handler: %s", handlerType.name());
        return nullptr;
    }

    return itr->second;
}

void SystemSubscriber::registerSystem(const SystemRegistration& sysReg)
{
    // Create subscriptions from the subscription requests by finding the desired component containers
    std::vector<ComponentSubscriptions> subscriptions;
    subscriptions.reserve(sysReg.subReqs.size());

    for (const ComponentSubReq& subReq : sysReg.subReqs) {
        const std::vector<ComponentUpdateReg>& componentRegs = subReq.componentTypes;

        ComponentSubscriptions newSub;
        newSub.componentTypes.reserve(componentRegs.size());

        newSub.subscriber = subReq.subscriber;
        newSub.onEntityAdded = subReq.onEntityAdded;

        for (const ComponentUpdateReg& componentReg : componentRegs) {
            auto queryItr = m_ComponentStorage.find(componentReg.tid);

            if (queryItr == m_ComponentStorage.end()) {
                Logger::LOG_WARNING("Attempted to subscribe to unregistered component type: %s, hash: %d", componentReg.tid.name(), componentReg.tid.hash_code());
                return;
            }

            newSub.componentTypes.push_back(componentReg.tid);
        }

        subscriptions.push_back(newSub);
    }

    if (subscriptions.size() == 0) {
        Logger::LOG_WARNING("No subscriptions were able to be made during a system registration");
        return;
    }

    size_t sysID = systemIdGen.genID();
    sysReg.system->ID = sysID;
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
        auto queryItr = m_ComponentStorage.find(subscription.componentTypes.front());

        // It has already been ensured that the component type is registered, no need to check for a missed search
        const IDContainer* pComponentContainer = queryItr->second.m_pContainer;
        const std::vector<Entity>& entities = pComponentContainer->getIDs();

        // See which entities in the entity vector also have all the other component types. Register those entities in the system.
        for (Entity entity : entities) {
            bool registerEntity = m_pEntityRegistry->entityHasTypes(entity, subscription.componentTypes);

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

        // TODO: Remove? Is it even possible for a system to have acquired an entity ID, and then have its subscription triggered again?
        if (sysSub.subscriber->hasElement(entityID)) {
            subBucketItr++;
            continue;
        }

        // Whether or not the entity has all the subscribed component types
        bool triggerSub = m_pEntityRegistry->entityHasTypes(entityID, sysSub.componentTypes);

        if (triggerSub) {
            sysSub.subscriber->push_back(entityID, entityID);

            if (sysSub.onEntityAdded != nullptr) {
                sysSub.onEntityAdded(entityID);
            }
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
