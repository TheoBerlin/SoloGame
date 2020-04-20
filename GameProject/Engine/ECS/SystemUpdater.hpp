#pragma once

// TODO: Move to config file
// TODO: Get maximum number of threads at runtime
#define MAX_THREADS 4

#include <Engine/ECS/System.hpp>
#include <Engine/Utils/IDGenerator.hpp>
#include <mutex>
#include <unordered_map>

struct SystemUpdateInfo {
    System* pSystem;
    std::vector<ComponentAccess> Components;
};

typedef std::vector<std::unordered_multimap<std::type_index, ComponentPermissions>::iterator> ProcessingSystemsIterators;

class SystemUpdater
{
public:
    SystemUpdater();
    ~SystemUpdater();

    void registerSystem(const SystemRegistration& sysReg);
    void deregisterSystem(System* pSystem);

    // Updates all systems with a single thread
    void updateST(float dt);
    // Updates all systems with multiple threads
    void updateMT(float dt);

private:
    IDGenerator m_SystemIDGen;

    // Contains pointers to systems as well as information on what components they process and with what permissions
    // so that safe multi-threading can be performed
    IDDVector<SystemUpdateInfo> m_UpdateInfos;

    /* Resources for multi-threaded updates below */
    // Stores what systems have been processed during a multi-threaded pass, where an element is an index to the vector of systems
    IDDVector<size_t> processedSystems;

    // Stores what components are currently being processed and with what rights
    std::unordered_multimap<std::type_index, ComponentPermissions> processingSystems;

    // Used when threads pick systems to update
    std::mutex mux;

    // Executed multiple threads simultaneously to continuously pick systems to update and update them until every one has been updated
    void updateSystems(float dt);

    const SystemUpdateInfo* findUpdateableSystem();

    /**
     * Registers a system's updated components types and their respective permissions
     * @param systemToRegister System to register as updating
     * @param processingSystemsIterators In/Out: Receives empty vector of iterators, inserts elements
     */
    void registerUpdate(const SystemUpdateInfo* systemToRegister, ProcessingSystemsIterators* processingSystemsIterators);
    void deregisterUpdate(const ProcessingSystemsIterators& processingSystemsIterators);

    // Used to time out threads unable to find updateable systems
    std::condition_variable timeoutCV;
    bool timeoutDisabled;
};
