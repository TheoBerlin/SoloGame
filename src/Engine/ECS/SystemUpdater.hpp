#pragma once

#include <Engine/ECS/System.hpp>
#include <Engine/Utils/IDGenerator.hpp>

#include <array>
#include <map>
#include <mutex>
#include <unordered_map>

// TODO: Get maximum number of threads at runtime
#define MAX_THREADS 4

const size_t g_UpdateQueueCount = 2;
const size_t g_LastUpdateQueue = g_UpdateQueueCount - 1;

struct SystemUpdateInfo {
    System* pSystem;
    std::vector<ComponentAccess> Components;
};

typedef IDDVector<SystemUpdateInfo> UpdateQueue;

typedef std::vector<std::unordered_map<std::type_index, size_t>::iterator> ProcessingSystemsIterators;

class SystemUpdater
{
public:
    SystemUpdater() = default;
    ~SystemUpdater() = default;

    void registerSystem(const SystemRegistration& sysReg);
    void deregisterSystem(System* pSystem);

    // Updates all systems with a single thread
    void updateST(float dt) const;
    // Updates all systems with multiple threads
    void updateMT(float dt);

private:
    static void registerComponentAccesses(const std::vector<ComponentAccess>& componentAccesses, std::map<std::type_index, ComponentPermissions>& uniqueRegs);

private:
    // Executed multiple threads simultaneously to continuously pick systems to update and update them until every one has been updated
    void updateSystems(const UpdateQueue& updateQueue, float dt);

    const SystemUpdateInfo* findUpdateableSystem(const UpdateQueue& updateQueue);

    /**
     * Registers a system's updated components types and their respective permissions
     * @param systemToRegister System to register as updating
     * @param processingSystemsIterators In/Out: Receives empty vector of iterators, inserts elements
     */
    void registerUpdate(const SystemUpdateInfo* systemToRegister, ProcessingSystemsIterators* processingSystemsIterators);
    void deregisterUpdate(const ProcessingSystemsIterators& processingSystemsIterators);

private:
    IDGenerator m_SystemIDGen;

    std::vector<size_t> m_ThreadHandles;

    // Each queue contains pointers to systems as well as information on what components they process and with what permissions
    // so that safe multi-threading can be performed
    std::array<UpdateQueue, g_UpdateQueueCount> m_UpdateQueues;

    /* Resources for multi-threaded updates below */
    // Stores what systems have been processed during a multi-threaded pass, where an element is an index to the vector of systems
    IDDVector<size_t> m_ProcessedSystems;

    // Maps component TIDs to the amount of systems are reading from them
    // Stores what components are currently being processed and with what rights
    std::unordered_map<std::type_index, size_t> m_ProcessingComponents;

    // Used when threads pick systems to update
    std::mutex m_Mux;

    // Used to time out threads unable to find updateable systems
    std::condition_variable m_TimeoutCV;
};
