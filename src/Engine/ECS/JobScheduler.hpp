#pragma once

#include <Engine/ECS/Job.hpp>

#include <condition_variable>
#include <mutex>
#include <unordered_set>

#define CURRENT_PHASE UINT32_MAX

class JobScheduler
{
public:
    JobScheduler();
    ~JobScheduler() = default;

    void update();

    void scheduleJob(const Job& job, uint32_t phase = CURRENT_PHASE);
    void scheduleJobs(const std::vector<Job>& jobs, uint32_t phase = CURRENT_PHASE);

    /*  scheduleRegularJob schedules a job that is performed each frame, until it is explicitly deregistered using the job ID.
        Returns the job ID. */
    size_t scheduleRegularJob(const Job& job, uint32_t phase = CURRENT_PHASE);
    void descheduleRegularJob(size_t phase, size_t jobID);

    // Schedules an advance to the next phase
    void nextPhase();

private:
    const Job* findExecutableJob();
    bool canExecute(const Job& job) const;

    // Register the component accesses (reads and writes) to be performed by the job
    void registerJobExecution(const Job& job);
    void deregisterJobExecution(const Job& job);

    void setPhase(uint32_t phase);

    // Checks if there are more jobs to execute in the current phase
    bool phaseJobsExist() const;

private:
    // One vector of jobs per phase. These jobs are anything but system updates, i.e. they are irregularly scheduled.
    // g_PhaseCount + 1 is used to allow jobs to be scheduled as post-systems
    std::array<std::vector<Job>, g_PhaseCount + 1> m_Jobs;

    // Indices to jobs yet to be executed within each phase
    std::array<std::vector<size_t>, g_PhaseCount + 1> m_JobIndices;

    // Regular jobs are performed each frame, until they are explicitly deregistered
    std::array<IDDVector<Job>, g_PhaseCount> m_RegularJobs;
    IDGenerator m_RegularJobIDGenerator;

    // IDs of the systems to update in the current phase
    // Populated at the beginning of each phase
    std::vector<size_t> m_RegularJobIDsToUpdate;

    // Maps component TIDs to the amount of jobs are reading from them.
    // A zero read count means the component type is being written to.
    std::unordered_map<std::type_index, size_t> m_ProcessingComponents;

    // Used to join threads executing jobs once the current phase's jobs have all been scheduled
    std::vector<size_t> m_ThreadHandles;

    std::mutex m_Lock;
    std::condition_variable m_ScheduleTimeoutCvar;

    uint32_t m_CurrentPhase;
};
