#include "SystemUpdater.hpp"

#include <Engine/ECS/System.hpp>
#include <Engine/Utils/Logger.hpp>

#include <thread>

SystemUpdater::SystemUpdater()
    :m_TimeoutDisabled(true)
{}

SystemUpdater::~SystemUpdater()
{}

void SystemUpdater::registerSystem(const SystemRegistration& sysReg)
{
    // Eliminate duplicate component types across the system's subscriptions
    std::map<std::type_index, ComponentPermissions> uniqueRegs;

    for (const ComponentSubscriptionRequest& subReq : sysReg.SubscriberRegistration.ComponentSubscriptionRequests) {
        SystemUpdater::registerComponentAccesses(subReq.m_ComponentAccesses, uniqueRegs);
    }

    SystemUpdater::registerComponentAccesses(sysReg.SubscriberRegistration.AdditionalDependencies, uniqueRegs);

    // Merge all of the system's subscribed component types into one vector
    std::vector<ComponentAccess> updateRegs;
    updateRegs.reserve(uniqueRegs.size());
    for (auto& uniqueRegsItr : uniqueRegs) {
        updateRegs.push_back({uniqueRegsItr.second, uniqueRegsItr.first});
    }

    size_t systemID = m_SystemIDGen.genID();
    System* pSystem = sysReg.pSystem;
    pSystem->setSystemID(systemID);

    SystemUpdateInfo sysUpdateInfo;
    sysUpdateInfo.pSystem = pSystem;
    sysUpdateInfo.Components = updateRegs;

    m_UpdateQueues[sysReg.UpdateQueueIndex].push_back(sysUpdateInfo, systemID);
}

void SystemUpdater::deregisterSystem(System* pSystem)
{
    bool foundSystem = false;
    size_t systemID = pSystem->getSystemID();

    for (UpdateQueue& updateQueue : m_UpdateQueues) {
        if (updateQueue.hasElement(systemID)) {
            updateQueue.pop(systemID);
            foundSystem = true;
            break;
        }
    }

    if (foundSystem) {
        m_SystemIDGen.popID(pSystem->getSystemID());
    } else {
        LOG_ERRORF("Failed to deregister system with ID: %ld", systemID);
    }
}

void SystemUpdater::updateST(float dt) const
{
    for (const UpdateQueue& updateQueue : m_UpdateQueues) {
        const std::vector<SystemUpdateInfo>& updateInfos = updateQueue.getVec();

        for (const SystemUpdateInfo& sysUpdateInfo : updateInfos) {
            sysUpdateInfo.pSystem->update(dt);
        }
    }
}

void SystemUpdater::updateMT(float dt)
{
    std::thread threads[MAX_THREADS];

    for (const UpdateQueue& updateQueue : m_UpdateQueues) {
        if (updateQueue.empty()) {
            continue;
        }

        std::generate(&threads[0], &threads[MAX_THREADS],
            [&]() { return std::thread(&SystemUpdater::updateSystems, this, updateQueue, dt); });

        for (std::thread& thread : threads) {
            thread.join();
        }

        m_ProcessedSystems.clear();
        m_ProcessingComponents.clear();
    }
}

void SystemUpdater::registerComponentAccesses(const std::vector<ComponentAccess>& componentAccesses, std::map<std::type_index, ComponentPermissions>& uniqueRegs)
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

void SystemUpdater::updateSystems(const UpdateQueue& updateQueue, float dt)
{
    std::unique_lock<std::mutex> lk(m_Mux);

    while (updateQueue.size() != m_ProcessedSystems.size()) {
        const SystemUpdateInfo* systemToUpdate = findUpdateableSystem(updateQueue);

        if (systemToUpdate == nullptr) {
            // No updateable system was found, have the thread wait until an update finishes
            m_TimeoutDisabled = false;
            m_TimeoutCV.wait(lk, [this]{ return m_TimeoutDisabled; });
        } else {
            ProcessingSystemsIterators updateIterators;
            registerUpdate(systemToUpdate, &updateIterators);

            m_Mux.unlock();
            systemToUpdate->pSystem->update(dt);
            m_Mux.lock();

            m_TimeoutDisabled = true;
            m_TimeoutCV.notify_all();

            deregisterUpdate(updateIterators);
        }
    }
}

const SystemUpdateInfo* SystemUpdater::findUpdateableSystem(const UpdateQueue& updateQueue)
{
    for (const SystemUpdateInfo& sysReg : updateQueue.getVec()) {
        // Check that the system hasn't already been processed
        if (m_ProcessedSystems.hasElement(sysReg.pSystem->getSystemID())) {
            continue;
        }

        bool systemIsEligible = true;

        // Prevent multiple systems from accessing the same components where at least one of them has write permissions
        for (const ComponentAccess& componentReg : sysReg.Components) {
            auto processingComponentItr = m_ProcessingComponents.find(componentReg.TID);
            if (processingComponentItr != m_ProcessingComponents.end() && (componentReg.Permissions == RW || processingComponentItr->second == 0)) {
                // The system wants to write to an already handled component type, or the component type is already being written to
                systemIsEligible = false;
                break;
            }
        }

        if (systemIsEligible) {
            return &sysReg;
        }
    }

    return nullptr;
}

void SystemUpdater::registerUpdate(const SystemUpdateInfo* systemToRegister, ProcessingSystemsIterators* processingSystemsIterators)
{
    size_t systemID = systemToRegister->pSystem->getSystemID();
    m_ProcessedSystems.push_back(systemID, systemID);

    processingSystemsIterators->reserve(systemToRegister->Components.size());

    for (const ComponentAccess& componentReg : systemToRegister->Components) {
        size_t isReading = componentReg.Permissions == R ? 1 : 0;

        auto processingComponentItr = m_ProcessingComponents.find(componentReg.TID);
        if (processingComponentItr == m_ProcessingComponents.end()) {
            processingSystemsIterators->push_back(m_ProcessingComponents.insert({componentReg.TID, isReading}).first);
        } else {
            processingSystemsIterators->push_back(processingComponentItr);
            processingComponentItr->second += isReading;
        }
    }
}

void SystemUpdater::deregisterUpdate(const ProcessingSystemsIterators& processingSystemsIterators)
{
    for (auto itr : processingSystemsIterators) {
        if (itr->second <= 1) {
            // The system the only one reading or writing to the component type
            m_ProcessingComponents.erase(itr);
        } else {
            itr->second -= 1;
        }
    }
}
