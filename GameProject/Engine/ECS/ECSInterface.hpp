#pragma once

#include <Engine/ECS/SystemSubscriber.hpp>
#include <Engine/ECS/SystemUpdater.hpp>
#include <Engine/Utils/IDGenerator.hpp>

struct ECSInterface {
    SystemSubscriber systemSubscriber;
    SystemUpdater systemUpdater;
    IDGenerator entityIDGen;
};
