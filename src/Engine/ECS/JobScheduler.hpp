#pragma once

#include "Engine/ECS/Job.hpp"
#include "Engine/Utils/IDGenerator.hpp"

#include <array>
#include <condition_variable>
#include <mutex>
#include <unordered_set>

class JobScheduler
{
public:
    JobScheduler();
    ~JobScheduler() = default;

    void Update(float dt);

    void ScheduleJob(const Job& job, uint32_t phase);
    void ScheduleJobASAP(const Job& job);
    void ScheduleJobs(const std::vector<Job>& jobs, uint32_t phase);

    /*  ScheduleRegularJob schedules a job that is performed each frame, until it is explicitly deregistered using the job ID.
        Returns the job ID. */
    uint32_t ScheduleRegularJob(const RegularJob& job, uint32_t phase);
    void DescheduleRegularJob(uint32_t phase, uint32_t jobID);

    const std::array<IDDVector<RegularJob>, PHASE_COUNT>& GetRegularJobs() const { return m_RegularJobs; }

private:
    const Job* FindExecutableJob();
    bool CanExecute(const Job& job) const;
    bool CanExecuteRegularJob(const RegularJob& job) const;
    void ExecuteJob(Job job);

    // Register the component accesses (reads and writes) to be performed by the job
    void RegisterJobExecution(const Job& job);
    void DeregisterJobExecution(const Job& job);

    void SetPhase(uint32_t phase);
    /*  Advances to the next phase. Performs component registrations and deletions and entity deletions. If it's the last phase already,
        the next phase might be a previous one, to accumulate regular jobs. */
    void NextPhase();

    void AccumulateRegularJobs();

    // Checks if there are more jobs to execute in the current phase
    bool PhaseJobsExist() const;

private:
    /*  One vector of jobs per phase. These jobs are anything but system ticks, i.e. they are irregularly scheduled.
        PHASE_COUNT + 1 is used to allow jobs to be scheduled as post-systems. */
    std::array<std::vector<Job>, PHASE_COUNT + 1> m_Jobs;

    // Indices to jobs yet to be executed within each phase
    std::array<std::vector<uint32_t>, PHASE_COUNT + 1> m_JobIndices;

    // Regular jobs are performed each frame, until they are explicitly deregistered
    std::array<IDDVector<RegularJob>, PHASE_COUNT> m_RegularJobs;
    IDGenerator m_RegularJobIDGenerator;

    /*  IDs of the regular job to tick in the current phase.
        Populated at the beginning of each phase. */
    std::array<std::vector<uint32_t>, PHASE_COUNT> m_RegularJobIDsToTick;
    /*  IDs of regular jobs in each phase that, regardless of whether they have been ticked, should be ticked
        at least one more time before the JobScheduler::Update exits. */
    std::array<std::unordered_set<uint32_t>, PHASE_COUNT> m_RegularJobsToAccumulate;

    /*  Maps component TIDs to the amount of jobs are reading from them.
        A zero read count means the component type is being written to. */
    std::unordered_map<const ComponentType*, uint32_t> m_ProcessingComponents;

    // Used to join threads executing jobs once the current phase's jobs have all been scheduled
    std::vector<uint32_t> m_ThreadHandles;

    std::mutex m_Lock;
    std::condition_variable m_ScheduleTimeoutCvar;

    uint32_t m_CurrentPhase;

    // The latest delta time retrieved through JobScheduler::Update
    float m_DeltaTime;
};
