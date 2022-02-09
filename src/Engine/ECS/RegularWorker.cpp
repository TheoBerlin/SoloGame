#include "RegularWorker.hpp"

#include "Engine/ECS/ECSCore.hpp"

RegularWorker::~RegularWorker()
{
	ECSCore* pECS = ECSCore::GetInstance();
	if (pECS && m_JobID != UINT32_MAX) {
		pECS->DescheduleRegularJob(m_Phase, m_JobID);
	}
}

void RegularWorker::Update()
{
	const float deltaTime = m_TickPeriod > 0.0f ? m_TickPeriod : ECSCore::GetInstance()->GetDeltaTime();
	m_TickFunction(deltaTime);
}

void RegularWorker::ScheduleRegularWork(const RegularWorkInfo& regularWorkInfo)
{
	m_Phase = regularWorkInfo.Phase;
	m_TickPeriod = regularWorkInfo.TickPeriod;
	m_TickFunction = regularWorkInfo.TickFunction;

	const RegularJob regularJob = {
		/* Components */	RegularWorker::GetUniqueComponentAccesses(regularWorkInfo.EntitySubscriberRegistration),
		/* Function */		std::bind(&RegularWorker::Update, this),
		/* TickPeriod */	m_TickPeriod,
		/* Accumulator */	0.0f
	};

	m_JobID = ECSCore::GetInstance()->ScheduleRegularJob(regularJob, m_Phase);
}

std::vector<ComponentAccess> RegularWorker::GetUniqueComponentAccesses(const EntitySubscriberRegistration& subscriberRegistration)
{
	// Eliminate duplicate component types across the system's subscriptions
	std::unordered_map<const ComponentType*, ComponentPermissions> uniqueRegs;

	for (const EntitySubscriptionRegistration& subReq : subscriberRegistration.EntitySubscriptionRegistrations) {
		RegularWorker::MapComponentAccesses(subReq.ComponentAccesses, uniqueRegs);
	}

	RegularWorker::MapComponentAccesses(subscriberRegistration.AdditionalAccesses, uniqueRegs);

	// Merge all of the system's subscribed component types into one vector
	std::vector<ComponentAccess> componentAccesses;
	componentAccesses.reserve((uint32_t)uniqueRegs.size());
	for (auto& uniqueRegsItr : uniqueRegs) {
		componentAccesses.push_back({uniqueRegsItr.second, uniqueRegsItr.first});
	}

	return componentAccesses;
}

void RegularWorker::MapComponentAccesses(const std::vector<ComponentAccess>& componentAccesses, std::unordered_map<const ComponentType*, ComponentPermissions>& uniqueRegs)
{
	for (const ComponentAccess& componentUpdateReg : componentAccesses) {
		if (componentUpdateReg.Permissions == NDA) {
			continue;
		}

		auto uniqueRegsItr = uniqueRegs.find(componentUpdateReg.pTID);
		if (uniqueRegsItr == uniqueRegs.end() || componentUpdateReg.Permissions > uniqueRegsItr->second) {
			uniqueRegs.insert(uniqueRegsItr, {componentUpdateReg.pTID, componentUpdateReg.Permissions});
		}
	}
}
