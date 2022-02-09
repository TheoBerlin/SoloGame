#pragma once

#include "Engine/ECS/EntitySubscriber.hpp"

#define PHASE_COUNT 4u
#define LAST_PHASE PHASE_COUNT - 1u

struct Job
{
	std::vector<ComponentAccess> Components;
	std::function<void()> Function;
};

struct RegularJob : Job
{
	float TickPeriod;
	float Accumulator;
};
