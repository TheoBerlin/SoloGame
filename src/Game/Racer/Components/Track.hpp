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

struct TrackSpeed {
    float Speed;
};

const std::type_index g_TIDTrackPosition    = TID(TrackPosition);
const std::type_index g_TIDTrackSpeed       = TID(TrackSpeed);

class TrackHandler : public ComponentHandler
{
public:
    TrackHandler(ECSCore* pECS);
    ~TrackHandler();

    virtual bool initHandler() override;

    void createTrackPosition(Entity entity);
    void createTrackSpeed(Entity entity, float speed = 0.0f);

    TrackPosition& getTrackPosition(Entity entity)   { return m_TrackPositions.indexID(entity); }
    float& getTrackSpeed(Entity entity)              { return m_TrackSpeeds.indexID(entity).Speed; }

private:
    IDDVector<TrackPosition> m_TrackPositions;
    IDDVector<TrackSpeed> m_TrackSpeeds;
};
