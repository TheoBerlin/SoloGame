#include "ECSCore.hpp"

ECSCore::ECSCore()
    :m_ComponentPublisher(&m_EntityRegistry),
    m_SystemRegistry(&m_DeltaTime),
    m_JobScheduler(&m_SystemRegistry),
    m_ECSBooter(this),
    m_DeltaTime(0.0f)
{}

void ECSCore::update(float dt)
{
    m_DeltaTime = dt;

    performDeletions();
    performRegistrations();

    m_JobScheduler.update();
}

void ECSCore::scheduleJobASAP(const Job& job)
{
    m_JobScheduler.scheduleJob(job, CURRENT_PHASE);
}

void ECSCore::scheduleJobPostFrame(const Job& job)
{
    m_JobScheduler.scheduleJob(job, g_LastPhase + 1u);
}

void ECSCore::enqueueComponentHandlerRegistration(const ComponentHandlerRegistration& handlerRegistration)
{
    m_ECSBooter.enqueueComponentHandlerRegistration(handlerRegistration);
}

void ECSCore::enqueueComponentSubscriberRegistration(const Subscriber& subscriber)
{
    m_ECSBooter.enqueueSubscriberInitialization(subscriber);
}

void ECSCore::performRegistrations()
{
    // Initialize and register systems and component handlers
    m_ECSBooter.performBootups();
}

void ECSCore::deleteEntityDelayed(Entity entity)
{
    m_EntitiesToDelete.push_back(entity);
}

void ECSCore::performDeletions()
{
    std::unordered_map<std::type_index, ComponentStorage>& componentStorage = m_ComponentPublisher.getComponentStorage();
    const EntityRegistryPage& registryPage = m_EntityRegistry.getTopRegistryPage();

    for (Entity entity : m_EntitiesToDelete) {
        // Delete every component belonging to the entity
        const auto& componentTypes = registryPage.indexID(entity);
        for (std::type_index componentType : componentTypes) {
            // Delete the component
            auto containerItr = componentStorage.find(componentType);
            ComponentStorage& component = containerItr->second;

            if (component.m_ComponentDestructor != nullptr) {
                component.m_ComponentDestructor(entity);
            }

            component.m_pContainer->pop(entity);

            // Notify systems that the component has been removed
            m_ComponentPublisher.removedComponent(entity, componentType);
        }

        // Free the entity ID
        m_EntityRegistry.deregisterEntity(entity);
    }

    m_EntitiesToDelete.clear();
}

void ECSCore::addRegistryPage()
{
    m_EntityRegistry.addPage();
}

void ECSCore::deregisterTopRegistryPage()
{
    const EntityRegistryPage& page = m_EntityRegistry.getTopRegistryPage();

    const auto& entityComponentSets = page.getVec();
    const std::vector<Entity>& entities = page.getIDs();

    for (size_t i = 0; i < entities.size(); i++) {
        const std::unordered_set<std::type_index>& typeSet = entityComponentSets[i];

        for (std::type_index type : typeSet) {
            // Deregister entity's components from systems
            m_ComponentPublisher.removedComponent(entities[i], type);
        }
    }
}

void ECSCore::deleteTopRegistryPage()
{
    const EntityRegistryPage& page = m_EntityRegistry.getTopRegistryPage();
    const auto& entityComponentSets = page.getVec();
    const std::vector<Entity>& entities = page.getIDs();

    std::unordered_map<std::type_index, ComponentStorage>& componentStorage = m_ComponentPublisher.getComponentStorage();

    for (size_t i = 0; i < entities.size(); i++) {
        const std::unordered_set<std::type_index>& typeSet = entityComponentSets[i];

        for (std::type_index componentType : typeSet) {
            // Deregister entity's component from systems
            m_ComponentPublisher.removedComponent(entities[i], componentType);

            // Delete the component
            auto containerItr = componentStorage.find(componentType);
            ComponentStorage& component = containerItr->second;

            if (component.m_ComponentDestructor != nullptr) {
                component.m_ComponentDestructor(entities[i]);
            }

            component.m_pContainer->pop(entities[i]);
        }
    }

    m_EntityRegistry.removePage();
}

void ECSCore::reinstateTopRegistryPage()
{
    const EntityRegistryPage& page = m_EntityRegistry.getTopRegistryPage();

    const auto& entityComponentSets = page.getVec();
    const std::vector<Entity>& entities = page.getIDs();

    for (size_t i = 0; i < entities.size(); i++) {
        const std::unordered_set<std::type_index>& typeSet = entityComponentSets[i];

        for (std::type_index componentType : typeSet) {
            m_ComponentPublisher.newComponent(entities[i], componentType);
        }
    }
}

void ECSCore::componentAdded(Entity entity, std::type_index componentType)
{
    m_EntityRegistry.registerComponentType(entity, componentType);
    m_ComponentPublisher.newComponent(entity, componentType);
}

void ECSCore::componentDeleted(Entity entity, std::type_index componentType)
{
    m_EntityRegistry.deregisterComponentType(entity, componentType);
    m_ComponentPublisher.removedComponent(entity, componentType);
}

void ECSCore::enqueueEntitySubscriptions(const EntitySubscriberRegistration& subscriberRegistration, const std::function<bool()>& initFn, size_t* pSubscriberID)
{
    Subscriber subscriber = {
        .ComponentSubscriptions = subscriberRegistration,
        .InitFunction           = initFn,
        .pSubscriberID          = pSubscriberID
    };

    m_ECSBooter.enqueueSubscriberInitialization(subscriber);
}
