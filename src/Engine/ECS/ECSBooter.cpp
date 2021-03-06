#include "ECSBooter.hpp"

#include <Engine/ECS/ComponentHandler.hpp>
#include <Engine/ECS/ECSCore.hpp>
#include <Engine/Rendering/Renderer.hpp>
#include <Engine/ECS/System.hpp>
#include <Engine/Utils/Logger.hpp>

ECSBooter::ECSBooter(ECSCore* pECS)
    :m_pECS(pECS)
{}

void ECSBooter::enqueueComponentHandlerRegistration(const ComponentHandlerRegistration& handlerRegistration)
{
    m_ComponentHandlersToRegister.push_back(handlerRegistration);
}

void ECSBooter::enqueueSubscriberInitialization(const Subscriber& subscriber)
{
    m_Subscribers.push_back(subscriber);
}

void ECSBooter::performBootups()
{
    if (m_ComponentHandlersToRegister.empty() && m_Subscribers.empty()) {
        return;
    }

    bootHandlers();
    bootSubscribers();
}

void ECSBooter::bootHandlers()
{
    buildBootInfos();
    finalizeHandlerBootDependencies();

    auto currentHandlerItr = m_HandlersBootInfos.begin();

    // Contains iterators to the current handler's dependencies
    std::vector<std::unordered_map<std::type_index, ComponentHandlerBootInfo>::iterator> openList;
    openList.reserve(m_HandlersBootInfos.size());

    // Outer loop: goes through handlers in m_HandlersToBoot until each handler has been booted
    while (currentHandlerItr != m_HandlersBootInfos.end()) {
        // Inner loop: recursively goes through the current handler's dependencies until it is booted
        auto dependencyItr = currentHandlerItr;

        while (!currentHandlerItr->second.inClosedList) {
            ComponentHandlerBootInfo& bootInfo = dependencyItr->second;
            if (bootInfo.inOpenList) {
                if (dependencyItr != openList.back()) {
                    LOG_ERRORF("Detected cyclic dependencies between component handlers, can not boot: %s", currentHandlerItr->first.name());
                    break;
                }
            } else {
                openList.push_back(dependencyItr);
                bootInfo.inOpenList = true;
            }

            // Check if the handler has any unbooted dependencies
            auto unbootedDependencyItr = std::find_if(bootInfo.dependencyMapIterators.begin(), bootInfo.dependencyMapIterators.end(),
                [](auto dependencyItr) -> bool { return !dependencyItr->second.inClosedList; });

            if (unbootedDependencyItr == bootInfo.dependencyMapIterators.end()) {
                // All dependencies are booted, boot the handler
                bootHandler(bootInfo);
                openList.pop_back();

                if (!openList.empty()) {
                    dependencyItr = openList.back();
                }
            } else {
                // Not all dependencies are booted, try to boot a dependency
                dependencyItr = *unbootedDependencyItr;
            }
        }

        currentHandlerItr++;
        openList.clear();
    }

    m_ComponentHandlersToRegister.clear();
    m_HandlersBootInfos.clear();
}

void ECSBooter::bootSubscribers()
{
    EntityPublisher* pEntityPublisher = m_pECS->getEntityPublisher();

    for (const Subscriber& subscriber : m_Subscribers) {
        if (!subscriber.InitFunction()) {
            LOG_ERROR("Failed to initialize subscriber");
        } else {
            *subscriber.pSubscriberID = pEntityPublisher->subscribeToComponents(subscriber.ComponentSubscriptions);
        }
    }

    m_Subscribers.clear();
}

void ECSBooter::buildBootInfos()
{
    for (const ComponentHandlerRegistration& handlerReg : m_ComponentHandlersToRegister) {
        ComponentHandlerBootInfo bootInfo = {};
        bootInfo.handlerRegistration = &handlerReg;
        bootInfo.dependencyMapIterators.reserve(handlerReg.HandlerDependencies.size());
        bootInfo.inOpenList = false;
        bootInfo.inClosedList = false;

        // Map the handler's type index to its boot info
        m_HandlersBootInfos[handlerReg.pComponentHandler->getHandlerType()] = bootInfo;
    }
}

void ECSBooter::finalizeHandlerBootDependencies()
{
    auto bootInfoItr = m_HandlersBootInfos.begin();
    while (bootInfoItr != m_HandlersBootInfos.end()) {
        ComponentHandlerBootInfo& bootInfo = bootInfoItr->second;
        bool hasDependencies = true;

        for (const std::type_index& dependencyTID : bootInfo.handlerRegistration->HandlerDependencies) {
            // Find the dependency's index
            auto dependencyItr = m_HandlersBootInfos.find(dependencyTID);
            if (dependencyItr != m_HandlersBootInfos.end()) {
                // The dependency is enqueued for bootup, store its iterator
                bootInfo.dependencyMapIterators.push_back(dependencyItr);
            } else {
                // The dependency is not enqueued for bootup, it might already be booted
                if (!m_pECS->getEntityPublisher()->getComponentHandler(dependencyTID)) {
                    LOG_ERRORF("Cannot boot handler: %s, missing dependency: %s", bootInfo.handlerRegistration->pComponentHandler->getHandlerType().name(), dependencyTID.name());
                    hasDependencies = false;
                    break;
                }
            }
        }

        if (hasDependencies) {
            bootInfoItr++;
        } else {
            bootInfoItr = m_HandlersBootInfos.erase(bootInfoItr);
        }
    }
}

void ECSBooter::bootHandler(ComponentHandlerBootInfo& bootInfo)
{
    bootInfo.inOpenList = false;
    bootInfo.inClosedList = true;

    if (!bootInfo.handlerRegistration->pComponentHandler->initHandler()) {
        LOG_ERRORF("Failed to initialize component handler: %s", bootInfo.handlerRegistration->pComponentHandler->getHandlerType().name());
    } else {
        m_pECS->getEntityPublisher()->registerComponentHandler(*bootInfo.handlerRegistration);
    }
}
