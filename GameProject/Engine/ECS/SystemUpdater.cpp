#include "SystemUpdater.hpp"

#include <Engine/ECS/System.hpp>
#include <thread>

SystemUpdater::SystemUpdater()
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
        threads[i] = std::thread(&SystemUpdater::updateSystem, this, dt);
    }

    for (unsigned short i = 0; i < MAX_THREADS; i += 1) {
        threads[i].join();
    }

    processedSystems.clear();
    processingSystems.clear();
}

void SystemUpdater::updateSystem(float dt)
{
    mux.lock();

    // Check that not all systems have been processed
    while (updateInfos.size() != processedSystems.size()) {
        for (const SystemUpdateInfo& sysReg : updateInfos.getVec()) {

            // Check that the system hasn't already been processed
            if (!processedSystems.hasElement(sysReg.system->ID)) {
                // Prevent multiple systems from accessing the same components where at least one of them has write permissions
                for (const ComponentUpdateReg& componentReg : sysReg.components) {
                    auto mapItr = processingSystems.find(componentReg.tid);

                    if (mapItr != processingSystems.end() && (mapItr->second & componentReg.permissions) == RW) {
                        continue;
                    }
                }

                // Found a system to process
                processedSystems.push_back(sysReg.system->ID, sysReg.system->ID);

                std::vector<std::unordered_multimap<std::type_index, ComponentPermissions>::iterator> mapIterators;
                mapIterators.reserve(sysReg.components.size());

                for (const ComponentUpdateReg& componentReg : sysReg.components) {
                    mapIterators.push_back(processingSystems.insert({componentReg.tid, componentReg.permissions}));
                }

                mux.unlock();
                sysReg.system->update(dt);
                mux.lock();

                // The system has finished updating
                for (auto itr : mapIterators) {
                    processingSystems.erase(itr);
                }
            }
        }
    }

    mux.unlock();
}
