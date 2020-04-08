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
            LOG_WARNING("Attempted to register an already handled component type: %s", componentReg.tid.name());
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
            LOG_WARNING("Attempted to deregister a component handler for an unregistered component type: %s", componentType.name());
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
        LOG_WARNING("Attempted to deregister an unregistered component handler: %s", handler->getHandlerType().name());
        return;
    }

    componentHandlers.erase(handlerItr);
}

ComponentHandler* SystemSubscriber::getComponentHandler(const std::type_index& handlerType)
{
    auto itr = componentHandlers.find(handlerType);

    if (itr == componentHandlers.end()) {
        LOG_WARNING("Failed to retrieve component handler: %s", handlerType.name());
        return nullptr;
    }

    return itr->second;
}

size_t SystemSubscriber::subscribeToComponents(const std::vector<ComponentSubscriptionRequest>& subscriptionRequests)
{
    // Create subscriptions from the subscription requests by finding the desired component containers
    std::vector<ComponentSubscriptions> subscriptions;
    subscriptions.reserve(subscriptionRequests.size());

    for (const ComponentSubscriptionRequest& subReq : subscriptionRequests) {
        const std::vector<ComponentUpdateReg>& componentRegs = subReq.componentTypes;

        ComponentSubscriptions newSub;
        newSub.componentTypes.reserve(componentRegs.size());

        newSub.subscriber = subReq.subscriber;
        newSub.onEntityAdded = subReq.onEntityAdded;

        for (const ComponentUpdateReg& componentReg : componentRegs) {
            auto queryItr = m_ComponentStorage.find(componentReg.tid);

            if (queryItr == m_ComponentStorage.end()) {
                LOG_ERROR("Attempted to subscribe to unregistered component type: %s, hash: %d", componentReg.tid.name(), componentReg.tid.hash_code());
                return 0;
            }

            newSub.componentTypes.push_back(componentReg.tid);
        }

        subscriptions.push_back(newSub);
    }

    size_t subID = systemIdGen.genID();
    this->subscriptionStorage.push_back(subscriptions, subID);

    // Map each component type to its subscriptions
    const std::vector<ComponentSubscriptions>& subs = subscriptionStorage.indexID(subID);

    for (size_t subscriptionNr = 0; subscriptionNr < subs.size(); subscriptionNr += 1) {
        const std::vector<std::type_index>& componentTypes = subs[subscriptionNr].componentTypes;

        for (const std::type_index& componentType : componentTypes) {
            componentSubscriptions.insert({componentType, {subID, subscriptionNr}});
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

    return subID;
}

void SystemSubscriber::unsubscribeFromComponents(size_t subscriptionID, std::vector<std::type_index>& componentTypes)
{
    if (subscriptionStorage.hasElement(subscriptionID) == false) {
        LOG_WARNING("Attempted to deregistered an unregistered system, ID: %d", subscriptionID);
        return;
    }

    // Use the subscriptions to find and delete component subscriptions
    std::vector<ComponentSubscriptions>& subscriptions = subscriptionStorage.indexID(subscriptionID);

    for (const ComponentSubscriptions& subscription : subscriptions) {
        const std::vector<std::type_index>& componentTypes = subscription.componentTypes;

        for (const std::type_index& componentType : componentTypes) {
            auto subBucketItr = componentSubscriptions.find(componentType);

            if (subBucketItr == componentSubscriptions.end()) {
                LOG_WARNING("Attempted to delete non-existent component subscription");
                // The other component subscriptions might exist, so don't return
                continue;
            }

            // Find the subscription and delete it
            while (subBucketItr != componentSubscriptions.end() && subBucketItr->first == componentType) {
                if (subBucketItr->second.systemID == subscriptionID) {
                    componentSubscriptions.erase(subBucketItr);
                    break;
                }

                subBucketItr++;
            }
        }
    }

    // All component->subscription mappings have been deleted
    subscriptionStorage.pop(subscriptionID);

    // Recycle system iD
    systemIdGen.popID(subscriptionID);
}

void SystemSubscriber::newComponent(Entity entityID, std::type_index componentType)
{
    // Get all subscriptions for the component type by iterating through the unordered_map bucket
    auto subBucketItr = componentSubscriptions.find(componentType);

    while (subBucketItr != componentSubscriptions.end() && subBucketItr->first == componentType) {
        // Use indices stored in the component type -> component storage mapping to get the component subscription
        ComponentSubscriptions& sysSub = subscriptionStorage.indexID(subBucketItr->second.systemID)[subBucketItr->second.subIdx];

        if (m_pEntityRegistry->entityHasTypes(entityID, sysSub.componentTypes)) {
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
