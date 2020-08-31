#include "RegularWorker.hpp"

RegularWorker::RegularWorker(ECSCore* pECS)
    :m_pECS(pECS),
    m_Phase(UINT32_MAX),
    m_JobID(UINT64_MAX)
{}

RegularWorker::~RegularWorker()
{
    m_pECS->descheduleRegularJob(m_Phase, m_JobID);
}

void RegularWorker::scheduleRegularWork(const Job& job, uint32_t phase)
{
    m_Phase = phase;
    m_JobID = m_pECS->scheduleRegularJob(job, phase);
}

std::vector<ComponentAccess> RegularWorker::getUniqueComponentAccesses(const EntitySubscriberRegistration& subscriberRegistration)
{
    // Eliminate duplicate component types across the system's subscriptions
    std::unordered_map<std::type_index, ComponentPermissions> uniqueRegs;

    for (const EntitySubscriptionRegistration& subReq : subscriberRegistration.EntitySubscriptionRegistrations) {
        RegularWorker::mapComponentAccesses(subReq.m_ComponentAccesses, uniqueRegs);
    }

    RegularWorker::mapComponentAccesses(subscriberRegistration.AdditionalDependencies, uniqueRegs);

    // Merge all of the system's subscribed component types into one vector
    std::vector<ComponentAccess> componentAccesses;
    componentAccesses.reserve(uniqueRegs.size());
    for (auto& uniqueRegsItr : uniqueRegs) {
        componentAccesses.push_back({uniqueRegsItr.second, uniqueRegsItr.first});
    }

    return componentAccesses;
}

void RegularWorker::mapComponentAccesses(const std::vector<ComponentAccess>& componentAccesses, std::unordered_map<std::type_index, ComponentPermissions>& uniqueRegs)
{
    for (const ComponentAccess& componentUpdateReg : componentAccesses) {
        if (componentUpdateReg.Permissions == NDA) {
            continue;
        }

        auto uniqueRegsItr = uniqueRegs.find(componentUpdateReg.TID);
        if (uniqueRegsItr == uniqueRegs.end() || componentUpdateReg.Permissions > uniqueRegsItr->second) {
            uniqueRegs.insert(uniqueRegsItr, {componentUpdateReg.TID, componentUpdateReg.Permissions});
        }
    }
}
