#pragma once

#include <Engine/ECS/Job.hpp>
#include <Engine/Utils/IDVector.hpp>

#include <array>

class ECSCore;

// SystemRegistry is responsible for assigning system IDs and converting system registrations into jobs
class SystemRegistry
{
public:
    SystemRegistry(const float* pDeltaTime);
    ~SystemRegistry() = default;

    void registerSystem(const SystemRegistration& sysReg);
    void deregisterSystem(System* pSystem);

    inline IDDVector<Job>& getPhaseJobs(uint32_t phaseIdx) { return m_UpdateJobs[phaseIdx]; }

private:
    void convertRegistrationToJob(const SystemRegistration& sysReg, Job& job);

    // Writes component accesses to a map to avoid duplicates. Filters NDA accesses.
    static void mapComponentAccesses(const std::vector<ComponentAccess>& componentAccesses, std::unordered_map<std::type_index, ComponentPermissions>& uniqueRegs);

private:
    // Each system's update function is registered within a phase
    std::array<IDDVector<Job>, g_PhaseCount> m_UpdateJobs;

    IDGenerator m_SystemIDGen;

    const float* m_pDeltaTime;
};
