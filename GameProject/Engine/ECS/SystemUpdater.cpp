#include "SystemUpdater.hpp"

#include <Engine/ECS/System.hpp>

SystemUpdater::SystemUpdater()
{}

SystemUpdater::~SystemUpdater()
{}

void SystemUpdater::registerSystem(SystemRegistration* sysReg)
{
    // Extract data from SystemRegistration to form SystemUpdateInfo objects
    std::vector<ComponentUpdateReg> updateRegs;

    for (size_t i = 0; i < sysReg->subReqs.size(); i += 1) {
        for (size_t j = 0; j < sysReg->subReqs[j].componentTypes.size(); j += 1) {
            updateRegs.push_back(sysReg->subReqs[i].componentTypes[j]);
        }
    }

    SystemUpdateInfo sysUpdateInfo;
    sysUpdateInfo.system = sysReg->system;
    sysUpdateInfo.components = updateRegs;

    this->systemRegs.push_back(sysUpdateInfo, sysReg->system->ID);
}

void SystemUpdater::updateST(float dt)
{
    for (size_t i = 0; i < systemRegs.size(); i += 1) {
        systemRegs[i].system->update(dt);
    }
}
