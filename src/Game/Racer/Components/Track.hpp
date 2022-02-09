#pragma once

#include <Engine/ECS/Component.hpp>

struct TrackPositionComponent {
    DECL_COMPONENT(TrackPositionComponent);
    // Index to the point used as P1
    size_t section;
    float T;
    float distanceFromCenter;
};

struct TrackSpeedComponent {
    DECL_COMPONENT(TrackSpeedComponent);
    float Speed;
};
