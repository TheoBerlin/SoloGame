#pragma once

#include <Engine/ECS/EntitySubscriber.hpp>
#include <Engine/ECS/Job.hpp>

class ECSCore;

// RegularWorker schedules a regular job and deregisters it upon destruction
class RegularWorker
{
public:
    RegularWorker(ECSCore* pECS);
    ~RegularWorker();

    void scheduleRegularWork(const Job& job, uint32_t phase);

protected:
    // getUniqueComponentAccesses serializes all unique component accesses in an entity subscriber registration
    std::vector<ComponentAccess> getUniqueComponentAccesses(const EntitySubscriberRegistration& subscriberRegistration);

private:
    static void mapComponentAccesses(const std::vector<ComponentAccess>& componentAccesses, std::unordered_map<std::type_index, ComponentPermissions>& uniqueRegs);

private:
    ECSCore* m_pECS;
    uint32_t m_Phase;
    size_t m_JobID;
};
