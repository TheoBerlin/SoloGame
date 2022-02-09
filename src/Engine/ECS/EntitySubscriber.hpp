#pragma once

#include "Engine/ECS/Component.hpp"
#include "Engine/ECS/Entity.hpp"

#include <functional>

class IDVector;

struct EntitySubscriptionRegistration {
	IDVector* pSubscriber;
	std::vector<ComponentAccess> ComponentAccesses;
	std::vector<IComponentGroup*> ComponentGroups;
	std::vector<const ComponentType*> ExcludedComponentTypes;
	// Optional: Called after an entity is added due to the subscription
	std::function<void(Entity)> OnEntityAdded;
	// Optional: Called before an entity is removed
	std::function<void(Entity)> OnEntityRemoval;
};

// EntitySubscriberRegistration is a complete set of data required to register a component subscriber
struct EntitySubscriberRegistration {
	std::vector<EntitySubscriptionRegistration> EntitySubscriptionRegistrations;
	/*  AdditionalAccesses are components that the subscriber will process, but are not part of any subscriptions.
		The subscriber will not store the entities whose components it will process. */
	std::vector<ComponentAccess> AdditionalAccesses;
};

// EntitySubscriber deregisters its entity subscriptions at destruction
class EntitySubscriber
{
public:
	EntitySubscriber() = default;
	~EntitySubscriber();

	// SubscribeToEntities enqueues entity subscriptions. initFn is called when all dependencies have been initialized.
	void SubscribeToEntities(EntitySubscriberRegistration& subscriberRegistration);

private:
	// ProcessComponentGroups removes duplicate component access registrations
	static void ProcessComponentGroups(EntitySubscriptionRegistration& subscriptionRegistration);
	// ProcessExcludedTypes asserts that the same component type isn't both included and excluded in a subscription
	static void ProcessExcludedTypes(EntitySubscriptionRegistration& subscriptionRegistration);

private:
	uint32_t m_SubscriptionID = UINT32_MAX;
};
