#include "SystemUpdater.hpp"

#include <Engine/ECS/System.hpp>
#include <thread>

SystemUpdater::SystemUpdater()
    :timeoutDisabled(true)
{}

SystemUpdater::~SystemUpdater()
{}

void SystemUpdater::registerSystem(SystemRegistration* sysReg)
{
    // Extract data from SystemRegistration to create SystemUpdateInfo objects
    std::vector<ComponentUpdateReg> updateRegs;

    for (size_t i = 0; i < sysReg->subReqs.size(); i += 1) {
        for (size_t j = 0; j < sysReg->subReqs[i].componentTypes.size(); j += 1) {
            updateRegs.push_back(sysReg->subReqs[i].componentTypes[j]);
        }
    }

    SystemUpdateInfo sysUpdateInfo;
    sysUpdateInfo.system = sysReg->system;
    sysUpdateInfo.components = updateRegs;

    this->updateInfos.push_back(sysUpdateInfo, sysReg->system->ID);
}

void SystemUpdater::deregisterSystem(System* system)
{
    this->updateInfos.pop(system->ID);
}

void SystemUpdater::updateST(float dt)
{
    for (size_t i = 0; i < updateInfos.size(); i += 1) {
        updateInfos[i].system->update(dt);
    }
}

void SystemUpdater::updateMT(float dt)
{
    std::thread threads[MAX_THREADS];

    for (unsigned short i = 0; i < MAX_THREADS; i += 1) {
        threads[i] = std::thread(&SystemUpdater::updateSystems, this, dt);
    }

    for (unsigned short i = 0; i < MAX_THREADS; i += 1) {
        threads[i].join();
    }

    processedSystems.clear();
    processingSystems.clear();
}

void SystemUpdater::updateSystems(float dt)
{
    std::unique_lock<std::mutex> lk(mux);

    while (updateInfos.size() != processedSystems.size()) {
        const SystemUpdateInfo* systemToUpdate = findUpdateableSystem();

        if (systemToUpdate == nullptr) {
            // No updateable system was found, have the thread wait until an update finishes
            timeoutDisabled = false;
            timeoutCV.wait(lk, [this]{return timeoutDisabled;});
        } else {
            ProcessingSystemsIterators updateIterators;
            registerUpdate(systemToUpdate, &updateIterators);

            mux.unlock();
            systemToUpdate->system->update(dt);
            mux.lock();

            timeoutDisabled = true;
            timeoutCV.notify_all();

            deregisterUpdate(&updateIterators);
        }
    }
}

const SystemUpdateInfo* SystemUpdater::findUpdateableSystem()
{
    for (const SystemUpdateInfo& sysReg : updateInfos.getVec()) {
        // Check that the system hasn't already been processed
        if (processedSystems.hasElement(sysReg.system->ID)) {
            continue;
        }

        // Prevent multiple systems from accessing the same components where at least one of them has write permissions
        for (const ComponentUpdateReg& componentReg : sysReg.components) {
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
    processedSystems.push_back(systemToRegister->system->ID, systemToRegister->system->ID);

    processingSystemsIterators->reserve(systemToRegister->components.size());

    for (const ComponentUpdateReg& componentReg : systemToRegister->components) {
        processingSystemsIterators->push_back(processingSystems.insert({componentReg.tid, componentReg.permissions}));
    }
}

void SystemUpdater::deregisterUpdate(const ProcessingSystemsIterators* processingSystemsIterators)
{
    for (auto itr : *processingSystemsIterators) {
        processingSystems.erase(itr);
    }
}
