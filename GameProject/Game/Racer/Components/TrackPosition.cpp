#include "TrackPosition.hpp"

TrackPositionHandler::TrackPositionHandler(ECSCore* pECS)
    :ComponentHandler({tid_trackPosition}, pECS, TID(TrackPositionHandler))
{
    ComponentHandlerRegistration handlerReg = {};
    handlerReg.pComponentHandler = this;
    handlerReg.ComponentRegistrations = {
        {tid_trackPosition, &trackPositions}
    };

    this->registerHandler(handlerReg);
}

TrackPositionHandler::~TrackPositionHandler()
{}

bool TrackPositionHandler::init()
{
    return true;
}

void TrackPositionHandler::createTrackPosition(Entity entity)
{
    TrackPosition trackPosition;
    trackPosition.section = 0;
    trackPosition.T = 0.0f;
    trackPosition.distanceFromCenter = 0.0f;
    trackPositions.push_back(trackPosition, entity);

    this->registerComponent(entity, tid_trackPosition);
}
