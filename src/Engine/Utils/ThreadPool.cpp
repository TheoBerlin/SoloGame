#include "ThreadPool.hpp"

ThreadPool ThreadPool::s_Instance;

ThreadPool::ThreadPool()
    :m_TimeToTerminate(false)
{}

ThreadPool::~ThreadPool()
{
    JoinAll();

    m_ScheduleLock.lock();
    m_TimeToTerminate = true;
    m_JobsExist.notify_all();
    m_ScheduleLock.unlock();

    for (std::thread& thread : m_Threads) {
        thread.join();
    }

    for (JoinResources* pJoinResources : m_JoinResources) {
        delete pJoinResources;
    }
}

void ThreadPool::Init()
{
    // hardware_concurrency might return 0
    size_t hwConc = std::thread::hardware_concurrency();
    size_t threadCount = hwConc ? hwConc : MIN_THREADS;

    m_Threads.reserve(threadCount);
    for (size_t threadIdx = 0u; threadIdx < threadCount; threadIdx++) {
        m_Threads.push_back(std::thread(std::bind(&ThreadPool::WaitForJob, this)));
    }

    m_JoinResources.resize(m_Threads.size() * 2u);
    m_FreeJoinResourcesIndices.resize(m_JoinResources.size());

    for (size_t threadHandleIdx = 0u; threadHandleIdx < m_JoinResources.size(); threadHandleIdx++) {
        m_JoinResources[threadHandleIdx] = DBG_NEW JoinResources();
        m_JoinResources[threadHandleIdx]->FinishSignal = false;
        m_FreeJoinResourcesIndices[threadHandleIdx] = threadHandleIdx;
    }

    LOG_INFOF("Started thread pool with %ld threads", threadCount);
}

size_t ThreadPool::Execute(std::function<void()> job)
{
    m_ScheduleLock.lock();
    size_t joinResourceIdx = 0u;
    if (m_FreeJoinResourcesIndices.empty()) {
        // Create new join resources
        joinResourceIdx = m_JoinResources.size();
        m_JoinResources.push_back(DBG_NEW JoinResources());
    } else {
        joinResourceIdx = m_FreeJoinResourcesIndices.back();
        m_FreeJoinResourcesIndices.pop_back();
    }

    m_JoinResources[joinResourceIdx]->FinishSignal = false;
    m_Jobs.push({ job, joinResourceIdx });
    m_JobsExist.notify_one();
    m_ScheduleLock.unlock();

    return joinResourceIdx;
}

void ThreadPool::ExecuteDetached(std::function<void()> job)
{
    m_ScheduleLock.lock();
    m_Jobs.push({ job, UINT64_MAX });
    m_JobsExist.notify_one();
    m_ScheduleLock.unlock();
}

void ThreadPool::Join(size_t joinResourcesIndex)
{
    JoinResources* pJoinResources = m_JoinResources[joinResourcesIndex];
    std::unique_lock<std::mutex> uLock(pJoinResources->Mutex);
    pJoinResources->CondVar.wait(uLock, [pJoinResources]{ return pJoinResources->FinishSignal; });

    // Free the join resources
    m_ScheduleLock.lock();
    m_FreeJoinResourcesIndices.push_back(joinResourcesIndex);
    m_ScheduleLock.unlock();
}

void ThreadPool::JoinAll()
{
    // Wait for all jobs to finish
    std::unique_lock<std::mutex> uLock(m_ScheduleLock);
    m_JobsExist.wait(uLock, [this]{ return m_Jobs.empty() && m_FreeJoinResourcesIndices.size() == m_JoinResources.size(); });
}

void ThreadPool::WaitForJob()
{
    std::unique_lock<std::mutex> uLock(m_ScheduleLock);
    while (true) {
        m_JobsExist.wait(uLock, [this]{ return !m_Jobs.empty() || m_TimeToTerminate; });

        if (!m_Jobs.empty()) {
            ThreadJob threadJob = m_Jobs.front();
            m_Jobs.pop();

            m_ScheduleLock.unlock();
            threadJob.Function();
            m_ScheduleLock.lock();

            // Notify joining threads that the job is finished
            if (threadJob.JoinResourcesIndex != UINT64_MAX) {
                JoinResources* pJoinResources = m_JoinResources[threadJob.JoinResourcesIndex];

                pJoinResources->Mutex.lock();
                pJoinResources->FinishSignal = true;
                pJoinResources->CondVar.notify_all();
                pJoinResources->Mutex.unlock();
            }
        }

        if (m_TimeToTerminate) {
            break;
        }
    }
}
