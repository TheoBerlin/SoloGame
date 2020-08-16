#include "JobScheduler.hpp"

#include <Engine/ECS/SystemRegistry.hpp>
#include <Engine/Utils/ThreadPool.hpp>

#include <numeric>

JobScheduler::JobScheduler(SystemRegistry* pSystemRegistry)
    :m_pSystemRegistry(pSystemRegistry),
    m_CurrentPhase(0u)
{}

void JobScheduler::update()
{
    std::unique_lock<std::mutex> uLock(m_Lock);
    ThreadPool& threadPool = ThreadPool::getInstance();

    setPhase(0u);

    // m_CurrentPhase == g_PhaseCount means all sytems are finished, and only post-systems jobs are executed
    while (m_CurrentPhase <= g_PhaseCount) {
        const Job* pJob = nullptr;
        do {
            pJob = findExecutableJob();
            if (pJob) {
                registerJobExecution(*pJob);
                m_ThreadHandles.push_back(threadPool.execute([this, pJob]{
                    pJob->Function();
                    m_Lock.lock();
                    deregisterJobExecution(*pJob);
                    m_Lock.unlock();
                }));
            }
        } while(pJob);

        if (!phaseJobsExist()) {
            // No more jobs in the current phase, perhaps currently running jobs will schedule new ones
            m_Lock.unlock();
            for (size_t threadHandle : m_ThreadHandles) {
                threadPool.join(threadHandle);
            }

            m_ThreadHandles.clear();
            m_Lock.lock();

            if (!phaseJobsExist()) {
                setPhase(m_CurrentPhase + 1u);
            }
        } else {
            m_ScheduleTimeoutCvar.wait(uLock);
        }
    }
}

void JobScheduler::scheduleJob(const Job& job, uint32_t phase)
{
    m_Lock.lock();
    phase = phase == CURRENT_PHASE ? m_CurrentPhase : phase;
    m_Jobs[phase].push_back(job);
    m_JobIndices[phase].push_back(m_Jobs[phase].size() - 1u);

    m_ScheduleTimeoutCvar.notify_all();
    m_Lock.unlock();
}

void JobScheduler::scheduleJobs(const std::vector<Job>& jobs, uint32_t phase)
{
    m_Lock.lock();
    phase = phase == CURRENT_PHASE ? m_CurrentPhase : phase;

    // Push jobs
    size_t oldJobsCount = m_Jobs[phase].size();
    m_Jobs[phase].resize(oldJobsCount + jobs.size());
    std::copy_n(jobs.begin(), jobs.size(), &m_Jobs[phase][oldJobsCount]);

    // Push job indices
    std::vector<size_t>& jobIndices = m_JobIndices[phase];
    size_t oldIndicesCount = m_JobIndices[phase].size();
    m_JobIndices[phase].resize(oldIndicesCount + jobs.size());
    std::iota(&jobIndices[oldIndicesCount - 1u], &jobIndices.back(), oldJobsCount);

    m_ScheduleTimeoutCvar.notify_all();

    m_Lock.unlock();
}

const Job* JobScheduler::findExecutableJob()
{
    std::vector<Job>& scheduledJobs = m_Jobs[m_CurrentPhase];
    std::vector<size_t>& jobIndices = m_JobIndices[m_CurrentPhase];

    for (size_t& jobIndex : jobIndices) {
        const Job& job = scheduledJobs[jobIndex];
        if (canExecute(job)) {
            jobIndex = jobIndices.back();
            jobIndices.pop_back();

            return &job;
        }
    }

    if (m_CurrentPhase < g_PhaseCount) {
        const IDDVector<Job>& systemJobs = m_pSystemRegistry->getPhaseJobs(m_CurrentPhase);
        for (size_t& systemID : m_SystemIDsToUpdate) {
            const Job& job = systemJobs.indexID(systemID);
            if (canExecute(job)) {
                systemID = m_SystemIDsToUpdate.back();
                m_SystemIDsToUpdate.pop_back();

                return &job;
            }
        }
    }

    return nullptr;
}

bool JobScheduler::canExecute(const Job& job) const
{
    // Prevent multiple jobs from accessing the same components where at least one of them has write permissions
    // i.e. prevent data races
    for (const ComponentAccess& componentReg : job.Components) {
        auto processingComponentItr = m_ProcessingComponents.find(componentReg.TID);
        if (processingComponentItr != m_ProcessingComponents.end() && (componentReg.Permissions == RW || processingComponentItr->second == 0)) {
            return false;
        }
    }

    return true;
}

void JobScheduler::registerJobExecution(const Job& job)
{
    for (const ComponentAccess& componentReg : job.Components) {
        size_t isReadOnly = componentReg.Permissions == R;

        auto processingComponentItr = m_ProcessingComponents.find(componentReg.TID);
        if (processingComponentItr == m_ProcessingComponents.end()) {
            m_ProcessingComponents.insert({componentReg.TID, isReadOnly});
        } else {
            processingComponentItr->second += isReadOnly;
        }
    }
}

void JobScheduler::deregisterJobExecution(const Job& job)
{
    for (const ComponentAccess& componentReg : job.Components) {
        auto processingComponentItr = m_ProcessingComponents.find(componentReg.TID);
        if (processingComponentItr->second <= 1u) {
            // The job was the only one reading or writing to the component type
            m_ProcessingComponents.erase(processingComponentItr);
        } else {
            processingComponentItr->second -= 1u;
        }
    }

    m_ScheduleTimeoutCvar.notify_all();
}

void JobScheduler::setPhase(uint32_t phase)
{
    if (m_CurrentPhase < g_PhaseCount + 1) {
        m_Jobs[m_CurrentPhase].clear();
    }

    m_CurrentPhase = phase;

    if (m_CurrentPhase < g_PhaseCount) {
        const std::vector<size_t>& systemIDs = m_pSystemRegistry->getPhaseJobs(m_CurrentPhase).getIDs();
        m_SystemIDsToUpdate.resize(systemIDs.size());
        std::memcpy(m_SystemIDsToUpdate.data(), systemIDs.data(), sizeof(size_t) * systemIDs.size());
    }
}

bool JobScheduler::phaseJobsExist() const
{
    return !m_SystemIDsToUpdate.empty() || !m_JobIndices[m_CurrentPhase].empty();
}
