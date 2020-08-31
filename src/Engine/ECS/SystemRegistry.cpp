#include "SystemRegistry.hpp"

SystemRegistry::SystemRegistry(const float* pDeltaTime)
    :m_pDeltaTime(pDeltaTime)
{}

void SystemRegistry::registerSystem(const SystemRegistration& sysReg)
{
    size_t systemID = m_SystemIDGen.genID();
    System* pSystem = sysReg.pSystem;
    pSystem->setSystemID(systemID);

    Job job;
    convertRegistrationToJob(sysReg, job);
    m_UpdateJobs[sysReg.Phase].push_back(job, systemID);
}

void SystemRegistry::deregisterSystem(System* pSystem)
{
    size_t systemID = pSystem->getSystemID();

    for (IDDVector<Job>& updateQueue : m_UpdateJobs) {
        if (updateQueue.hasElement(systemID)) {
            updateQueue.pop(systemID);
            m_SystemIDGen.popID(pSystem->getSystemID());
            return;
        }
    }

    LOG_ERRORF("Failed to deregister system with ID: %ld", systemID);
}

void SystemRegistry::convertRegistrationToJob(const SystemRegistration& sysReg, Job& job)
{
    // Eliminate duplicate component types across the system's subscriptions
    std::unordered_map<std::type_index, ComponentPermissions> uniqueRegs;

    for (const EntitySubscriptionRegistration& subReq : sysReg.SubscriberRegistration.EntitySubscriptionRegistrations) {
        SystemRegistry::mapComponentAccesses(subReq.m_ComponentAccesses, uniqueRegs);
    }

    SystemRegistry::mapComponentAccesses(sysReg.SubscriberRegistration.AdditionalDependencies, uniqueRegs);

    // Merge all of the system's subscribed component types into one vector
    job.Components.reserve(uniqueRegs.size());
    for (auto& uniqueRegsItr : uniqueRegs) {
        job.Components.push_back({uniqueRegsItr.second, uniqueRegsItr.first});
    }

    System* pSystem = sysReg.pSystem;
    job.Function = [pSystem, this]{ pSystem->update(*m_pDeltaTime); };
}

void SystemRegistry::mapComponentAccesses(const std::vector<ComponentAccess>& componentAccesses, std::unordered_map<std::type_index, ComponentPermissions>& uniqueRegs)
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
