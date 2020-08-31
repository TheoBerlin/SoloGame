#pragma once

#include <Engine/ECS/ComponentHandler.hpp>
#include <Engine/ECS/EntityPublisher.hpp>
#include <Engine/ECS/ECSBooter.hpp>
#include <Engine/ECS/EntityRegistry.hpp>
#include <Engine/ECS/JobScheduler.hpp>
#include <Engine/Rendering/Renderer.hpp>
#include <Engine/ECS/System.hpp>
#include <Engine/Utils/IDGenerator.hpp>

#include <vector>

class EntitySubscriber;
class RegularWorker;

class ECSCore
{
public:
    ECSCore();
    ~ECSCore() = default;

    ECSCore(const ECSCore& other) = delete;
    void operator=(const ECSCore& other) = delete;

    void update(float dt);

    Entity createEntity() { return m_EntityRegistry.createEntity(); }

    void scheduleJobASAP(const Job& job);
    void scheduleJobPostFrame(const Job& job);

    void enqueueComponentHandlerRegistration(const ComponentHandlerRegistration& handlerRegistration);
    void enqueueComponentSubscriberRegistration(const Subscriber& subscriber);

    // Registers and initializes component handlers and entity subscribers
    void performRegistrations();

    // Enqueues an entity deletion
    void enqueueEntityDeletion(Entity entity);
    void performEntityDeletions();

    void addRegistryPage();
    void deregisterTopRegistryPage();
    void deleteTopRegistryPage();
    void reinstateTopRegistryPage();

    void componentAdded(Entity entity, std::type_index componentType);
    void componentDeleted(Entity entity, std::type_index componentType);

    EntityPublisher* getEntityPublisher()   { return &m_EntityPublisher; }
    float getDeltaTime()                    { return m_DeltaTime; }

protected:
    friend EntitySubscriber;
    void enqueueEntitySubscriptions(const EntitySubscriberRegistration& subscriberRegistration, const std::function<bool()>& initFn, size_t* pSubscriberID);

    friend RegularWorker;
    size_t scheduleRegularJob(const Job& job, uint32_t phase)   { return m_JobScheduler.scheduleRegularJob(job, phase); };
    void descheduleRegularJob(uint32_t phase, size_t jobID)     { m_JobScheduler.descheduleRegularJob(phase, jobID); };

private:
    EntityRegistry m_EntityRegistry;
    EntityPublisher m_EntityPublisher;
    JobScheduler m_JobScheduler;
    ECSBooter m_ECSBooter;

    std::vector<Entity> m_EntitiesToDelete;

    // DeltaTime is the time between frames. The typical 'dt' that is passed to update()
    float m_DeltaTime;
};
