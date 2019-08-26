#include "RacerMover.hpp"

#include <Engine/ECS/ECSInterface.hpp>
#include <Engine/Transform.hpp>
#include <Game/Level/Tube.hpp>
#include <Game/Racer/Components/TrackPosition.hpp>

#include <Engine/Utils/Logger.hpp>

RacerMover::RacerMover(ECSInterface* ecs)
    :System(ecs)
{
    std::type_index tid_transformHandler = std::type_index(typeid(TransformHandler));
    this->transformHandler = static_cast<TransformHandler*>(ecs->systemSubscriber.getComponentHandler(tid_transformHandler));

    std::type_index tid_trackPositionHandler= std::type_index(typeid(TrackPositionHandler));
    this->trackPositionHandler = static_cast<TrackPositionHandler*>(ecs->systemSubscriber.getComponentHandler(tid_trackPositionHandler));

    std::type_index tid_tubeHandler= std::type_index(typeid(TubeHandler));
    this->tubeHandler = static_cast<TubeHandler*>(ecs->systemSubscriber.getComponentHandler(tid_tubeHandler));

    SystemRegistration sysReg = {
    {
        {{{RW, tid_transform}, {RW, tid_trackPosition}}, &racers}
    },
    this};

    this->subscribeToComponents(&sysReg);
    this->registerUpdate(&sysReg);
}

RacerMover::~RacerMover()
{}

void RacerMover::update(float dt)
{
    for (size_t i = 0; i < racers.size(); i += 1) {
        Transform& transform = transformHandler->transforms.indexID(i);
        TrackPosition& trackPosition = trackPositionHandler->trackPositions.indexID(i);

        // Get four control points for catmull-rom
        size_t idx0, idx1, idx2, idx3;
        idx0 = std::max((int)trackPosition.section-1, 0);
        idx1 = trackPosition.section;
        idx2 = trackPosition.section+1;
        idx3 = std::min(trackPosition.section+2, tubeHandler->tubeSections.size()-1);

        DirectX::XMVECTOR P[4];
        P[0] = DirectX::XMLoadFloat3(&tubeHandler->tubeSections[idx0]);
        P[1] = DirectX::XMLoadFloat3(&tubeHandler->tubeSections[idx1]);
        P[2] = DirectX::XMLoadFloat3(&tubeHandler->tubeSections[idx2]);
        P[3] = DirectX::XMLoadFloat3(&tubeHandler->tubeSections[idx3]);

        // Calculate the distance between P1 and P2 to figure out by how much to increase T per second
        DirectX::XMVECTOR temp = DirectX::XMVector3Length(DirectX::XMVectorSubtract(P[2], P[1]));
        float sectionLengthReciprocal = 1.0f / DirectX::XMVectorGetX(temp);

        float tPerSecond = racerSpeed * sectionLengthReciprocal;

        unsigned hasNotReachedEnd = (trackPosition.section != tubeHandler->tubeSections.size() - 2) || (trackPosition.T < 1.0f);
        bool A = hasNotReachedEnd;
        trackPosition.T += tPerSecond * dt * hasNotReachedEnd;
        hasNotReachedEnd = (trackPosition.section != tubeHandler->tubeSections.size() - 2) || (trackPosition.T < 1.0f);

        size_t oldSection = trackPosition.section;

        float tFloor = 0.0f;
        float tFractal = std::modff(trackPosition.T, &tFloor);
        trackPosition.section += (size_t)tFloor * hasNotReachedEnd;

        // Floor T to 0, or 1 if the racer has reached the end
        trackPosition.T -= tFloor * hasNotReachedEnd;
        hasNotReachedEnd = (trackPosition.section != tubeHandler->tubeSections.size() - 2) || (trackPosition.T < 1.0f);

        if (oldSection != trackPosition.section) {
            Logger::LOG_INFO("Section %d->%d", oldSection, trackPosition.section);
        }

        if (A && !hasNotReachedEnd) {
            Logger::LOG_INFO("End reached: T: %f, Section: %d", trackPosition.T, trackPosition.section);
        }

        // TODO: if the racer reaches a new section, tPerSecond will change, account for this!

        // Update transform
        temp = DirectX::XMVectorCatmullRom(P[0], P[1], P[2], P[3], trackPosition.T);
        DirectX::XMStoreFloat3(&transform.position, temp);
    }
}
