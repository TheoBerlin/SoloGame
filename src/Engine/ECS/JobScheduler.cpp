#include "JobScheduler.hpp"

#include "Engine/ECS/ECSCore.hpp"
#include "Engine/ECS/EntitySubscriber.hpp"
#include "Engine/Utils/ThreadPool.hpp"

#include <numeric>

JobScheduler::JobScheduler() :
        m_CurrentPhase(0u)
    ,   m_DeltaTime(0.0f)
{}

void JobScheduler::Update(float dt)
{
    ThreadPool& threadPool = ThreadPool::GetInstance();

    m_DeltaTime = dt;

    std::unique_lock<std::mutex> uLock(m_Lock);
    SetPhase(0u);
    AccumulateRegularJobs();

    // m_CurrentPhase == PHASE_COUNT means all regular jobs are finished, and only post-systems jobs are executed
    while (m_CurrentPhase <= PHASE_COUNT) {
        const Job* pJob = nullptr;
        do {
            pJob = FindExecutableJob();
            if (pJob) {
                RegisterJobExecution(*pJob);
                m_ThreadHandles.push_back((uint32_t)threadPool.Execute(std::bind_front(&JobScheduler::ExecuteJob, this, *pJob)));
            }
        } while(pJob);

        if (!PhaseJobsExist()) {
            // No more jobs in the current phase, perhaps currently running jobs will schedule new ones
            m_Lock.unlock();
            for (uint32_t threadHandle : m_ThreadHandles) {
                threadPool.Join(threadHandle);
            }

            m_ThreadHandles.clear();
            m_Lock.lock();

            if (!PhaseJobsExist()) {
                NextPhase();
            }
        }
        else {
            m_ScheduleTimeoutCvar.wait(uLock);
        }
    }
}

void JobScheduler::ScheduleJob(const Job& job, uint32_t phase)
{
    std::scoped_lock<std::mutex> lock(m_Lock);

    m_Jobs[phase].emplace_back(job);
    m_JobIndices[phase].push_back((uint32_t)m_Jobs[phase].size() - 1u);

    m_ScheduleTimeoutCvar.notify_all();
}

void JobScheduler::ScheduleJobASAP(const Job& job)
{
    std::scoped_lock<std::mutex> lock(m_Lock);

    const uint32_t phase = m_CurrentPhase >= m_Jobs.size() ? 0u : m_CurrentPhase;
    m_Jobs[phase].emplace_back(job);
    m_JobIndices[phase].push_back((uint32_t)m_Jobs[phase].size() - 1u);

    m_ScheduleTimeoutCvar.notify_all();
}

void JobScheduler::ScheduleJobs(const std::vector<Job>& jobs, uint32_t phase)
{
    std::scoped_lock<std::mutex> lock(m_Lock);

    // Push jobs
    const size_t oldJobsCount = m_Jobs[phase].size();
    m_Jobs[phase].resize(oldJobsCount + jobs.size());
    std::copy_n(jobs.begin(), jobs.size(), &m_Jobs[phase][oldJobsCount]);

    // Push job indices
    std::vector<uint32_t>& jobIndices = m_JobIndices[phase];
    const size_t oldIndicesCount = m_JobIndices[phase].size();
    m_JobIndices[phase].resize(oldIndicesCount + jobs.size());
    std::iota(&jobIndices[oldIndicesCount - 1u], &jobIndices.back(), (uint32_t)oldJobsCount);

    m_ScheduleTimeoutCvar.notify_all();
}

uint32_t JobScheduler::ScheduleRegularJob(const RegularJob& job, uint32_t phase)
{
    std::scoped_lock<std::mutex> lock(m_Lock);

    const uint32_t jobID = m_RegularJobIDGenerator.GenID();
    m_RegularJobs[phase].push_back(job, jobID);

    return jobID;
}

void JobScheduler::DescheduleRegularJob(uint32_t phase, uint32_t jobID)
{
    std::scoped_lock<std::mutex> lock(m_Lock);
    m_RegularJobs[phase].Pop(jobID);
}

const Job* JobScheduler::FindExecutableJob()
{
    const std::vector<Job>& scheduledJobs = m_Jobs[m_CurrentPhase];
    std::vector<uint32_t>& jobIndices = m_JobIndices[m_CurrentPhase];

    for (uint32_t& jobIndex : jobIndices)
    {
        const Job& job = scheduledJobs[jobIndex];
        if (CanExecute(job))
        {
            jobIndex = jobIndices.back();
            jobIndices.pop_back();

            return &job;
        }
    }

    if (m_CurrentPhase < PHASE_COUNT && !m_RegularJobsToAccumulate[m_CurrentPhase].empty())
    {
        std::vector<uint32_t>& regularJobIDsToTick = m_RegularJobIDsToTick[m_CurrentPhase];
        for (uint32_t& jobID : regularJobIDsToTick)
        {
            RegularJob& job = m_RegularJobs[m_CurrentPhase].IndexID(jobID);

            if (CanExecute(job))
            {
                job.Accumulator -= job.TickPeriod;
                if (job.TickPeriod <= 0.0f || job.Accumulator < job.TickPeriod)
                {
                    m_RegularJobsToAccumulate[m_CurrentPhase].erase(jobID);
                }

                jobID = regularJobIDsToTick.back();
                regularJobIDsToTick.pop_back();

                return &job;
            }
        }
    }

    return nullptr;
}

bool JobScheduler::CanExecute(const Job& job) const
{
    // Prevent multiple jobs from accessing the same components where at least one of them has write permissions
    // i.e. prevent data races
    for (const ComponentAccess& componentReg : job.Components)
    {
        auto processingComponentItr = m_ProcessingComponents.find(componentReg.pTID);
        /*  processingComponentItr->second is the amount of jobs currently reading from the component type.
            If the count is zero, a job is writing to the component type. */
        if (processingComponentItr != m_ProcessingComponents.end() && (componentReg.Permissions == RW || processingComponentItr->second == 0))
        {
            return false;
        }
    }

    return true;
}

bool JobScheduler::CanExecuteRegularJob(const RegularJob& job) const
{
    const Job& simpleJob = job;
    return job.Accumulator >= job.TickPeriod && CanExecute(simpleJob);
}

void JobScheduler::ExecuteJob(Job job)
{
    job.Function();
    m_Lock.lock();
    DeregisterJobExecution(job);
    m_Lock.unlock();
}

void JobScheduler::RegisterJobExecution(const Job& job)
{
    for (const ComponentAccess& componentReg : job.Components)
    {
        const uint32_t isReadOnly = componentReg.Permissions == R;

        auto processingComponentItr = m_ProcessingComponents.find(componentReg.pTID);
        if (processingComponentItr == m_ProcessingComponents.end())
        {
            m_ProcessingComponents.insert({componentReg.pTID, isReadOnly});
        }
        else
        {
            processingComponentItr->second += isReadOnly;
        }
    }
}

void JobScheduler::DeregisterJobExecution(const Job& job)
{
    for (const ComponentAccess& componentReg : job.Components)
    {
        auto processingComponentItr = m_ProcessingComponents.find(componentReg.pTID);
        if (processingComponentItr->second <= 1u)
        {
            // The job was the only one reading or writing to the component type
            m_ProcessingComponents.erase(processingComponentItr);
        }
        else
        {
            processingComponentItr->second -= 1u;
        }
    }

    m_ScheduleTimeoutCvar.notify_all();
}

void JobScheduler::SetPhase(uint32_t phase)
{
    if (m_CurrentPhase <= PHASE_COUNT)
    {
        m_Jobs[m_CurrentPhase].clear();
    }

    m_CurrentPhase = phase;
}

void JobScheduler::NextPhase()
{
    uint32_t upcomingPhase = m_CurrentPhase + 1u;
    // All phases have been processed. Some might need to be replayed to accumulate regular jobs
    if (m_CurrentPhase == PHASE_COUNT)
    {
        for (uint32_t phase = 0; phase < PHASE_COUNT; phase++)
        {
            const std::unordered_set<uint32_t>& regularJobsToAccumulate = m_RegularJobsToAccumulate[phase];
            if (!regularJobsToAccumulate.empty())
            {
                // Find which regular jobs need accumulating
                std::vector<uint32_t>& regularJobIDsToTick = m_RegularJobIDsToTick[phase];
                for (uint32_t jobID : regularJobsToAccumulate)
                {
                    regularJobIDsToTick.push_back(jobID);
                }

                upcomingPhase = phase;
                break;
            }
        }
    }

    ECSCore* pECS = ECSCore::GetInstance();
    pECS->PerformComponentRegistrations();
    pECS->PerformComponentDeletions();
    pECS->PerformEntityDeletions();
    SetPhase(upcomingPhase);
}

void JobScheduler::AccumulateRegularJobs()
{
    for (uint32_t phase = 0; phase < PHASE_COUNT; phase++) {
        IDDVector<RegularJob>& regularJobs = m_RegularJobs[phase];
        const std::vector<uint32_t>& jobIDs = regularJobs.GetIDs();
        std::vector<uint32_t>& regularJobIDsToTick = m_RegularJobIDsToTick[phase];

        for (uint32_t jobIdx = 0; jobIdx < regularJobs.Size(); jobIdx++) {
            RegularJob& regularJob = regularJobs[jobIdx];
            // Increase the accumulator only if the regular job has a non-zero tick period
            regularJob.Accumulator += m_DeltaTime * (regularJob.TickPeriod > 0.0f);
            if (regularJob.Accumulator >= regularJob.TickPeriod) {
                m_RegularJobsToAccumulate[phase].insert(jobIDs[jobIdx]);
                regularJobIDsToTick.push_back(jobIDs[jobIdx]);
            }
        }
    }
}

bool JobScheduler::PhaseJobsExist() const
{
    return (m_CurrentPhase < PHASE_COUNT && !m_RegularJobIDsToTick[m_CurrentPhase].empty()) || !m_JobIndices[m_CurrentPhase].empty();
}
