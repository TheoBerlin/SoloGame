#pragma once

#include <Engine/ECS/ComponentSubscriptionRequest.hpp>
#include <Engine/ECS/Entity.hpp>
#include <Engine/Utils/IDVector.hpp>

#include <functional>
#include <typeindex>
#include <vector>

class System;

struct SystemRegistration {
    ComponentSubscriberRegistration SubscriberRegistration;
    System* pSystem;
    size_t UpdateQueueIndex = 0;
};

class ComponentHandler;
class ComponentSubscriber;
class ComponentSubscriptionRequest;
class ECSCore;

/*
    A system stores pointers to component storages and processes the components each frame
    in the update function
*/
class System
{
public:
    // Registers the system in the system handler
    System(ECSCore* pECS);

    // Deregisters system
    virtual ~System();

    virtual bool initSystem() = 0;

    virtual void update(float dt) = 0;

    inline size_t getSystemID() const { return m_SystemID; }
    void setSystemID(size_t systemID) { m_SystemID = systemID; }

    void setComponentSubscriptionID(size_t ID) { m_ComponentSubscriptionID = ID; }

protected:
    void subscribeToComponents(const SystemRegistration& sysReg);
    void registerUpdate(const SystemRegistration& sysReg);

    ComponentHandler* getComponentHandler(const std::type_index& handlerType);

private:
    ECSCore* m_pECS;

    size_t m_ComponentSubscriptionID;
    size_t m_SystemID;
};
