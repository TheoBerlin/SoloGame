#pragma once

// TODO: Move to config file
#define MAX_THREADS 4

#include <Engine/ECS/System.hpp>
#include <mutex>
#include <unordered_map>

struct SystemUpdateInfo {
    System* system;
    std::vector<ComponentUpdateReg> components;
};

class SystemUpdater
{
public:
    SystemUpdater();
    ~SystemUpdater();

    void registerSystem(SystemRegistration* sysReg);
    void deregisterSystem(System* system);

    // Updates all systems with a single thread
    void updateST(float dt);
    // Updates all systems with multiple threads
    void updateMT(float dt);

private:
    // Contains pointers to systems as well as information on what components they process and with what permissions
    // so that safe multi-threading can be performed
    IDVector<SystemUpdateInfo> updateInfos;

    /* Resources for multi-threaded updates below */
    // Stores what systems have been processed during a multi-threaded pass, where an element is an index to the vector of systems
    IDVector<size_t> processedSystems;

    // Stores what components are currently being processed and with what rights
    std::unordered_multimap<std::type_index, ComponentPermissions> processingSystems;

    // Used when threads pick systems to update
    std::mutex mux;

    // Executed by one of multiple threads to continuously pick systems to update and update them until every one has been updated
    void updateSystem(float dt);
};
