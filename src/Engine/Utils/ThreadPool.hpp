#pragma once

#include <condition_variable>
#include <functional>
#include <thread>
#include <vector>

// Used to block threads calling join(), and used by working threads to notify when the job is finished
struct JoinResources {
    std::mutex Mutex;
    std::condition_variable CondVar;
    bool FinishSignal;
};

struct ThreadJob {
    std::function<void()> Function;
    size_t JoinResourcesIndex; // UINT64_MAX is specified when join resources should not be used
};

// In case hardware_concurrency() returns 0, this is the default amount of threads the thread pool will start
#define MIN_THREADS 4u

class ThreadPool
{
public:
    ThreadPool();
    ~ThreadPool();

    ThreadPool(const ThreadPool& other) = delete;
    void operator=(const ThreadPool& other) = delete;

    static ThreadPool& getInstance() { return s_Instance; }

    void initialize();

    // Returns index to thread join resources. Calling join() on the returned index is required.
    size_t execute(std::function<void()> job);

    // Schedules a job without any join resources attached. Calling joinAll() before the program exits is required.
    void executeDetached(std::function<void()> job);

    void join(size_t joinResourcesIndex);
    void joinAll();

    size_t getThreadCount() const { return m_Threads.size(); }

private:
    // Infinite loop where threads wait for jobs
    void waitForJob();

private:
    static ThreadPool s_Instance;

    std::vector<std::thread> m_Threads;
    std::queue<ThreadJob> m_Jobs;
    std::condition_variable m_JobsExist;

    // Pool of join resources to each thread job. Expanded upon need.
    std::vector<JoinResources*> m_JoinResources;
    std::vector<size_t> m_FreeJoinResourcesIndices;
    std::mutex m_ScheduleLock;

    // Signals when threads should stop looking for jobs in order to delete the thread pool
    bool m_TimeToTerminate;
};
