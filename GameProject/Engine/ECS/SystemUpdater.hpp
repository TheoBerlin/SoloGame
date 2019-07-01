#pragma once

#include <Engine/ECS/System.hpp>
#include <vector>

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

    // Updates all systems with a single thread
    void updateST(float dt);
    // Updates all systems with multiple threads
    // TODO: void updateMT(float dt);

private:
    // Contains pointers to systems as well as information on what components they process and with what permissions
    // so that safe multi-threading can be performed
    IDVector<SystemUpdateInfo> systemRegs;

    // Stores what systems have been processed during a multi-threaded pass, where an element is an index to the vector of systems
    IDVector<size_t> processedSystems;
};
