#include "Track.hpp"

TrackHandler::TrackHandler(ECSCore* pECS)
    :ComponentHandler(pECS, TID(TrackHandler))
{
    ComponentHandlerRegistration handlerReg = {};
    handlerReg.pComponentHandler = this;
    handlerReg.ComponentRegistrations = {
        {g_TIDTrackPosition, &m_TrackPositions},
        {g_TIDTrackSpeed, &m_TrackSpeeds}
    };

    this->registerHandler(handlerReg);
}

TrackHandler::~TrackHandler()
{}

bool TrackHandler::initHandler()
{
    return true;
}

void TrackHandler::createTrackPosition(Entity entity)
{
    TrackPosition trackPosition;
    trackPosition.section = 0;
    trackPosition.T = 0.0f;
    trackPosition.distanceFromCenter = 0.0f;
    m_TrackPositions.push_back(trackPosition, entity);

    this->registerComponent(entity, g_TIDTrackPosition);
}

void TrackHandler::createTrackSpeed(Entity entity, float speed)
{
    TrackSpeed trackSpeed = {speed};
    m_TrackSpeeds.push_back(trackSpeed, entity);

    this->registerComponent(entity, g_TIDTrackSpeed);
}
