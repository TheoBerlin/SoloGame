#pragma once

#include <typeindex>
#include <queue>
#include <vector>
#include <unordered_map>

class ComponentHandler;
class ECSCore;
class JobScheduler;
struct ComponentHandlerRegistration;
struct RendererRegistration;
struct SystemRegistration;

struct ComponentHandlerBootInfo {
    const ComponentHandlerRegistration* handlerRegistration = nullptr;
    // Iterators that point out a handler's place in the boot info map
    std::vector<std::unordered_map<std::type_index, ComponentHandlerBootInfo>::iterator> dependencyMapIterators;
    // Used during bootup, to detect cyclic dependencies
    bool inOpenList = false, inClosedList = false;
};

struct Subscriber {
    EntitySubscriberRegistration ComponentSubscriptions;
    std::function<bool()> InitFunction;
    size_t* pSubscriberID;
};

class ECSBooter
{
public:
    ECSBooter(ECSCore* pECS);
    ~ECSBooter() = default;

    void enqueueComponentHandlerRegistration(const ComponentHandlerRegistration& handlerRegistration);
    void enqueueSubscriberInitialization(const Subscriber& subscriber);

    void performBootups();

private:
    struct OpenListInfo {
        ComponentHandlerBootInfo& bootInfo;
        bool inOpenList, inClosedList;
    };

private:
    void bootHandlers();
    void bootSubscribers();

    void buildBootInfos();
    // Get the map iterators of each handler's dependency, for faster lookups. Also remove dependencies that have already been booted.
    void finalizeHandlerBootDependencies();
    void bootHandler(ComponentHandlerBootInfo& bootInfo);

private:
    ECSCore* m_pECS;

    std::vector<ComponentHandlerRegistration> m_ComponentHandlersToRegister;
    std::vector<Subscriber> m_Subscribers;

    // Maps handlers' types to their boot info
    std::unordered_map<std::type_index, ComponentHandlerBootInfo> m_HandlersBootInfos;
};
