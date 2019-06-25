#pragma once

#include <Engine/ECS/SystemHandler.hpp>
#include <Engine/Utils/IDGenerator.hpp>

struct ECSInterface {
    SystemHandler systemHandler;
    IDGenerator entityIDGen;
};
