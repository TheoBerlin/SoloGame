#include "SystemUpdater.hpp"

#include <Engine/ECS/System.hpp>
#include <thread>

SystemUpdater::SystemUpdater()
    :timeoutDisabled(true)
{}

SystemUpdater::~SystemUpdater()
{}

void SystemUpdater::registerSystem(const SystemRegistration& sysReg)
{
    // Extract data from SystemRegistration to create SystemUpdateInfo objects
    std::vector<ComponentUpdateReg> updateRegs;

    for (const ComponentSubscriptionRequest& subReq : sysReg.SubscriptionRequests) {
        updateRegs.reserve(updateRegs.size() + subReq.componentTypes.size());
        for (const ComponentUpdateReg& componentUpdateReg : subReq.componentTypes) {
            updateRegs.push_back(componentUpdateReg);
        }
    }

    size_t systemID = m_SystemIDGen.genID();
    System* pSystem = sysReg.pSystem;
    pSystem->setSystemID(systemID);

    SystemUpdateInfo sysUpdateInfo;
    sysUpdateInfo.pSystem = pSystem;
    sysUpdateInfo.Components = updateRegs;

    this->m_UpdateInfos.push_back(sysUpdateInfo, systemID);
}

void SystemUpdater::deregisterSystem(System* pSystem)
{
    this->m_UpdateInfos.pop(pSystem->getSystemID());
    m_SystemIDGen.popID(pSystem->getSystemID());
}

void SystemUpdater::updateST(float dt)
{
    std::vector<SystemUpdateInfo>& updateInfos = m_UpdateInfos.getVec();

    for (SystemUpdateInfo& sysUpdateInfo : updateInfos) {
        sysUpdateInfo.pSystem->update(dt);
    }
}

void SystemUpdater::updateMT(float dt)
{
    std::thread threads[MAX_THREADS];

    for (std::thread& thread : threads) {
        thread = std::thread(&SystemUpdater::updateSystems, this, dt);
    }

    for (std::thread& thread : threads) {
        thread.join();
    }

    processedSystems.clear();
    processingSystems.clear();
}

void SystemUpdater::updateSystems(float dt)
{
    std::unique_lock<std::mutex> lk(mux);

    while (m_UpdateInfos.size() != processedSystems.size()) {
        const SystemUpdateInfo* systemToUpdate = findUpdateableSystem();

        if (systemToUpdate == nullptr) {
            // No updateable system was found, have the thread wait until an update finishes
            timeoutDisabled = false;
            timeoutCV.wait(lk, [this]{return timeoutDisabled;});
        } else {
            ProcessingSystemsIterators updateIterators;
            registerUpdate(systemToUpdate, &updateIterators);

            mux.unlock();
            systemToUpdate->pSystem->update(dt);
            mux.lock();

            timeoutDisabled = true;
            timeoutCV.notify_all();

            deregisterUpdate(updateIterators);
        }
    }
}

const SystemUpdateInfo* SystemUpdater::findUpdateableSystem()
{
    for (const SystemUpdateInfo& sysReg : m_UpdateInfos.getVec()) {
        // Check that the system hasn't already been processed
        if (processedSystems.hasElement(sysReg.pSystem->getSystemID())) {
            continue;
        }

        // Prevent multiple systems from accessing the same components where at least one of them has write permissions
        for (const ComponentUpdateReg& componentReg : sysReg.Components) {
            auto permissionsItrs = processingSystems.equal_range(componentReg.tid);

            while (permissionsItrs.first != permissionsItrs.second) {
                if ((permissionsItrs.second->second & componentReg.permissions) == RW) {
                    // System collides with another currently updating system
                    return nullptr;
                }

                permissionsItrs.first++;
            }
        }

        return &sysReg;
    }

    return nullptr;
}

void SystemUpdater::registerUpdate(const SystemUpdateInfo* systemToRegister, ProcessingSystemsIterators* processingSystemsIterators)
{
    size_t systemID = systemToRegister->pSystem->getSystemID();
    processedSystems.push_back(systemID, systemID);

    processingSystemsIterators->reserve(systemToRegister->Components.size());

    for (const ComponentUpdateReg& componentReg : systemToRegister->Components) {
        processingSystemsIterators->push_back(processingSystems.insert({componentReg.tid, componentReg.permissions}));
    }
}

void SystemUpdater::deregisterUpdate(const ProcessingSystemsIterators& processingSystemsIterators)
{
    for (auto itr : processingSystemsIterators) {
        processingSystems.erase(itr);
    }
}
