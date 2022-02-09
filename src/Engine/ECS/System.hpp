#pragma once

#include "Engine/ECS/Entity.hpp"
#include "Engine/ECS/EntitySubscriber.hpp"
#include "Engine/ECS/RegularWorker.hpp"
#include "Engine/Utils/IDVector.hpp"

#include <functional>
#include <typeindex>

struct SystemRegistration {
	EntitySubscriberRegistration SubscriberRegistration;
	uint32_t Phase = 0;
	uint32_t TickFrequency = 0;
};

// A system processes components in the Update function
class System : public EntitySubscriber, RegularWorker
{
public:
	~System() = default;

	virtual void Update(float deltaTime) = 0;

	const std::string& GetName() const { return m_SystemName; }

protected:
	virtual void RegisterSystem(const std::string& systemName, SystemRegistration& systemRegistration);

private:
	std::string m_SystemName;
};
