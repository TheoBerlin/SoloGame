#include "TrackPosition.hpp"

TrackPositionHandler::TrackPositionHandler(ECSCore* pECS)
    :ComponentHandler({tid_trackPosition}, pECS, std::type_index(typeid(TrackPositionHandler)))
{
    std::vector<ComponentRegistration> compRegs = {
        {tid_trackPosition, &trackPositions}
    };

    this->registerHandler(&compRegs);
}

TrackPositionHandler::~TrackPositionHandler()
{}

void TrackPositionHandler::createTrackPosition(Entity entity)
{
    TrackPosition trackPosition;
    trackPosition.section = 0;
    trackPosition.T = 0.0f;
    trackPosition.distanceFromCenter = 0.0f;
    trackPositions.push_back(trackPosition, entity);

    this->registerComponent(entity, tid_trackPosition);
}
