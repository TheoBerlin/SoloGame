#pragma once

#include <Engine/ECS/ComponentHandler.hpp>
#include <Engine/Utils/IDVector.hpp>
#include <typeindex>

struct TrackPosition {
    // Index to the point used as P1
    size_t section;
    float T;
};

const std::type_index tid_trackPosition = std::type_index(typeid(TrackPosition));

class TrackPositionHandler : public ComponentHandler
{
public:
    TrackPositionHandler(SystemSubscriber* systemSubscriber);
    ~TrackPositionHandler();

    void createTrackPosition(Entity entity);

    IDVector<TrackPosition> trackPositions;
};
