#include "ComponentPublisher.hpp"

#include <Engine/ECS/ComponentHandler.hpp>
#include <Engine/ECS/System.hpp>
#include <Engine/Utils/GeneralUtils.hpp>
#include <Engine/Utils/Logger.hpp>

ComponentPublisher::ComponentPublisher(EntityRegistry* pEntityRegistry)
    :m_pEntityRegistry(pEntityRegistry)
{}

ComponentPublisher::~ComponentPublisher()
{}


void ComponentPublisher::registerComponentHandler(const ComponentHandlerRegistration& componentHandlerRegistration)
{
    ComponentHandler* pComponentHandler = componentHandlerRegistration.pComponentHandler;
    std::type_index tidHandler = pComponentHandler->getHandlerType();

    this->m_ComponentHandlers[tidHandler] = pComponentHandler;

    // Register component containers
    for (const ComponentRegistration& componentReg : componentHandlerRegistration.ComponentRegistrations) {
        auto mapItr = this->m_ComponentStorage.find(componentReg.tid);

        if (mapItr != this->m_ComponentStorage.end()) {
            LOG_WARNINGF("Attempted to register an already handled component type: %s", componentReg.tid.name());
            continue;
        }

        this->m_ComponentStorage.insert({componentReg.tid, {componentReg.pComponentContainer, componentReg.m_ComponentDestructor}});
    }
}

void ComponentPublisher::deregisterComponentHandler(ComponentHandler* handler)
{
    const std::vector<std::type_index>& componentTypes = handler->getHandledTypes();

    for (const std::type_index& componentType : componentTypes) {
        // Delete component query functions
        auto handlerItr = m_ComponentStorage.find(componentType);

        if (handlerItr == m_ComponentStorage.end()) {
            LOG_WARNINGF("Attempted to deregister a component handler for an unregistered component type: %s", componentType.name());
            continue;
        }

        const std::vector<Entity>& entities = handlerItr->second.m_pContainer->getIDs();
        for (Entity entity : entities) {
            removedComponent(entity, componentType);
        }

        m_ComponentStorage.erase(handlerItr);
    }

    auto handlerItr = m_ComponentHandlers.find(handler->getHandlerType());

    if (handlerItr == m_ComponentHandlers.end()) {
        LOG_WARNINGF("Attempted to deregister an unregistered component handler: %s", handler->getHandlerType().name());
        return;
    }

    m_ComponentHandlers.erase(handlerItr);
}

ComponentHandler* ComponentPublisher::getComponentHandler(const std::type_index& handlerType)
{
    auto itr = m_ComponentHandlers.find(handlerType);

    if (itr == m_ComponentHandlers.end()) {
        LOG_WARNINGF("Failed to retrieve component handler: %s", handlerType.name());
        return nullptr;
    }

    return itr->second;
}

size_t ComponentPublisher::subscribeToComponents(const EntitySubscriberRegistration& subscriberRegistration)
{
    // Create subscriptions from the subscription requests by finding the desired component containers
    std::vector<ComponentSubscriptions> subscriptions;
    const std::vector<EntitySubscriptionRegistration>& subscriptionRequests = subscriberRegistration.EntitySubscriptionRegistrations;
    subscriptions.reserve(subscriptionRequests.size());

    for (const EntitySubscriptionRegistration& subReq : subscriptionRequests) {
        const std::vector<ComponentAccess>& componentRegs = subReq.m_ComponentAccesses;

        ComponentSubscriptions newSub;
        newSub.componentTypes.reserve(componentRegs.size());

        newSub.subscriber = subReq.m_pSubscriber;
        newSub.onEntityAdded = subReq.m_OnEntityAdded;
        newSub.onEntityRemoved = subReq.m_OnEntityRemoved;

        for (const ComponentAccess& componentReg : componentRegs) {
            auto queryItr = m_ComponentStorage.find(componentReg.TID);

            if (queryItr == m_ComponentStorage.end()) {
                LOG_ERRORF("Attempted to subscribe to unregistered component type: %s, hash: %d", componentReg.TID.name(), componentReg.TID.hash_code());
                return 0;
            }

            newSub.componentTypes.push_back(componentReg.TID);
        }

        eliminateDuplicates(newSub.componentTypes);
        newSub.componentTypes.shrink_to_fit();
        subscriptions.push_back(newSub);
    }

    size_t subID = m_SystemIDGenerator.genID();
    m_SubscriptionStorage.push_back(subscriptions, subID);

    // Map each component type to its subscriptions
    const std::vector<ComponentSubscriptions>& subs = m_SubscriptionStorage.indexID(subID);

    for (size_t subscriptionNr = 0; subscriptionNr < subs.size(); subscriptionNr += 1) {
        const std::vector<std::type_index>& componentTypes = subs[subscriptionNr].componentTypes;

        for (const std::type_index& componentType : componentTypes) {
            m_ComponentSubscriptions.insert({componentType, {subID, subscriptionNr}});
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
                subscription.subscriber->push_back(entity);

                if (subscription.onEntityAdded != nullptr) {
                    subscription.onEntityAdded(entity);
                }
            }
        }
    }

    return subID;
}

void ComponentPublisher::unsubscribeFromComponents(size_t subscriptionID)
{
    if (m_SubscriptionStorage.hasElement(subscriptionID) == false) {
        LOG_WARNINGF("Attempted to deregistered an unregistered system, ID: %d", subscriptionID);
        return;
    }

    // Use the subscriptions to find and delete component subscriptions
    std::vector<ComponentSubscriptions>& subscriptions = m_SubscriptionStorage.indexID(subscriptionID);

    for (const ComponentSubscriptions& subscription : subscriptions) {
        const std::vector<std::type_index>& componentTypes = subscription.componentTypes;

        for (const std::type_index& componentType : componentTypes) {
            auto subBucketItr = m_ComponentSubscriptions.find(componentType);

            if (subBucketItr == m_ComponentSubscriptions.end()) {
                LOG_WARNING("Attempted to delete non-existent component subscription");
                // The other component subscriptions might exist, so don't return
                continue;
            }

            // Find the subscription and delete it
            while (subBucketItr != m_ComponentSubscriptions.end() && subBucketItr->first == componentType) {
                if (subBucketItr->second.systemID == subscriptionID) {
                    m_ComponentSubscriptions.erase(subBucketItr);
                    break;
                }

                subBucketItr++;
            }
        }
    }

    // All component->subscription mappings have been deleted
    m_SubscriptionStorage.pop(subscriptionID);

    // Recycle system iD
    m_SystemIDGenerator.popID(subscriptionID);
}

void ComponentPublisher::newComponent(Entity entityID, std::type_index componentType)
{
    // Get all subscriptions for the component type by iterating through the unordered_map bucket
    auto subBucketItr = m_ComponentSubscriptions.find(componentType);

    while (subBucketItr != m_ComponentSubscriptions.end() && subBucketItr->first == componentType) {
        // Use indices stored in the component type -> component storage mapping to get the component subscription
        ComponentSubscriptions& sysSub = m_SubscriptionStorage.indexID(subBucketItr->second.systemID)[subBucketItr->second.subIdx];

        if (m_pEntityRegistry->entityHasTypes(entityID, sysSub.componentTypes)) {
            sysSub.subscriber->push_back(entityID);

            if (sysSub.onEntityAdded) {
                sysSub.onEntityAdded(entityID);
            }
        }

        subBucketItr++;
    }
}

void ComponentPublisher::removedComponent(Entity entityID, std::type_index componentType)
{
    // Get all subscriptions for the component type by iterating through the unordered_map bucket
    auto subBucketItr = m_ComponentSubscriptions.find(componentType);

    while (subBucketItr != m_ComponentSubscriptions.end() && subBucketItr->first == componentType) {
        // Use indices stored in the component type -> component storage mapping to get the component subscription
        ComponentSubscriptions& sysSub = m_SubscriptionStorage.indexID(subBucketItr->second.systemID)[subBucketItr->second.subIdx];

        if (!sysSub.subscriber->hasElement(entityID)) {
            subBucketItr++;
            continue;
        }

        if (sysSub.onEntityRemoved) {
            sysSub.onEntityRemoved(entityID);
        }

        sysSub.subscriber->pop(entityID);

        subBucketItr++;
    }
}
