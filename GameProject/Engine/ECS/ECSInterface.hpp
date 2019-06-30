#pragma once

#include <Engine/ECS/SystemSubscriber.hpp>
#include <Engine/Utils/IDGenerator.hpp>

struct ECSInterface {
    SystemSubscriber systemSubscriber;
    IDGenerator entityIDGen;
};
