#pragma once

#include <Engine/ECS/EntitySubscriber.hpp>
#include <Engine/ECS/Entity.hpp>
#include <Engine/Utils/IDVector.hpp>

#include <functional>
#include <typeindex>
#include <vector>

class System;

struct SystemRegistration {
    EntitySubscriberRegistration SubscriberRegistration;
    System* pSystem;
    size_t Phase = 0;
};

class ComponentHandler;
class ECSCore;

// A system processes components each frame in the update function
class System : private EntitySubscriber
{
public:
    // Registers the system in the system handler
    System(ECSCore* pECS);

    // Deregisters system
    virtual ~System() = default;

    virtual bool initSystem() = 0;

    virtual void update(float dt) = 0;

    size_t getSystemID() const          { return m_SystemID; }
    void setSystemID(size_t systemID)   { m_SystemID = systemID; }

protected:
    void subscribeToComponents(const SystemRegistration& sysReg);
    void enqueueRegistration(const SystemRegistration& sysReg);

    ComponentHandler* getComponentHandler(const std::type_index& handlerType);

private:
    ECSCore* m_pECS;
    size_t m_SystemID;
};
