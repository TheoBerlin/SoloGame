#pragma once

#include <Engine/ECS/ComponentHandler.hpp>
#include <Engine/Utils/IDVector.hpp>
#include <Engine/Utils/ECSUtils.hpp>
#include <typeindex>

struct TrackPosition {
    // Index to the point used as P1
    size_t section;
    float T;
    float distanceFromCenter;
};

const std::type_index tid_trackPosition = TID(TrackPosition);

class TrackPositionHandler : public ComponentHandler
{
public:
    TrackPositionHandler(ECSCore* pECS);
    ~TrackPositionHandler();

    virtual bool initHandler() override;

    void createTrackPosition(Entity entity);

    IDDVector<TrackPosition> trackPositions;
};
