#pragma once

#include <Engine/ECS/EntitySubscriber.hpp>
#include <Engine/ECS/Job.hpp>

struct RegularWorkInfo {
	std::function<void(float deltaTime)> TickFunction;
	EntitySubscriberRegistration EntitySubscriberRegistration;
	uint32_t Phase;
	float TickPeriod;
};

// RegularWorker schedules a regular job and deregisters it upon destruction
class RegularWorker
{
public:
	RegularWorker() = default;
	~RegularWorker();

	void Update();

	void ScheduleRegularWork(const RegularWorkInfo& regularWorkInfo);

protected:
	uint32_t GetJobID() const { return m_JobID; }

protected:
	// GetUniqueComponentAccesses serializes all unique component accesses in an entity subscriber registration
	static std::vector<ComponentAccess> GetUniqueComponentAccesses(const EntitySubscriberRegistration& subscriberRegistration);

private:
	static void MapComponentAccesses(const std::vector<ComponentAccess>& componentAccesses, std::unordered_map<const ComponentType*, ComponentPermissions>& uniqueRegs);

private:
	uint32_t m_Phase = UINT32_MAX;
	uint32_t m_JobID = UINT32_MAX;

	float m_TickPeriod = -1.0f;

	std::function<void(float deltaTime)> m_TickFunction = nullptr;
};
