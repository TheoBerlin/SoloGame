#include "RacerMover.hpp"

#include <Engine/ECS/ECSInterface.hpp>
#include <Engine/Transform.hpp>
#include <Game/Level/Tube.hpp>
#include <Game/Racer/Components/TrackPosition.hpp>

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
    const std::vector<DirectX::XMFLOAT3>& tubeSections = tubeHandler->getTubeSections();

    for (size_t i = 0; i < racers.size(); i += 1) {
        Transform& transform = transformHandler->transforms.indexID(i);
        TrackPosition& trackPosition = trackPositionHandler->trackPositions.indexID(i);

        // Calculate the distance between P1 and P2 to figure out by how much to increase T per second
        DirectX::XMVECTOR P[4];
        P[1] = DirectX::XMLoadFloat3(&tubeSections[trackPosition.section]);
        P[2] = DirectX::XMLoadFloat3(&tubeSections[trackPosition.section+1]);

        DirectX::XMVECTOR temp = DirectX::XMVector3Length(DirectX::XMVectorSubtract(P[2], P[1]));
        float sectionLength = DirectX::XMVectorGetX(temp);
        float tPerSecond = racerSpeed / sectionLength;

        unsigned hasNotReachedEnd = (trackPosition.section != tubeSections.size() - 2) || (trackPosition.T < 1.0f);
        trackPosition.T += tPerSecond * dt * hasNotReachedEnd;
        hasNotReachedEnd = (trackPosition.section != tubeSections.size() - 2) || (trackPosition.T < 1.0f);

        // Advance in sections and clamp T to [0, 1) if the racer has not reached the end
        while (std::floorf(trackPosition.T) > 0.0f && hasNotReachedEnd) {
            trackPosition.section += 1;

            // Update T by accounting for the next section's T/s
            P[1] = DirectX::XMLoadFloat3(&tubeSections[trackPosition.section]);
            P[2] = DirectX::XMLoadFloat3(&tubeSections[trackPosition.section+1]);

            temp = DirectX::XMVector3Length(DirectX::XMVectorSubtract(P[2], P[1]));
            float newSectionLength = DirectX::XMVectorGetX(temp);

            trackPosition.T = (newSectionLength / sectionLength) * (trackPosition.T - 1.0f);
            hasNotReachedEnd = (trackPosition.section != tubeSections.size() - 2) || (trackPosition.T < 1.0f);
        }

        // P[1] and P[2] are loaded since earlier
        size_t idx0, idx3;
        idx0 = std::max((int)trackPosition.section-1, 0);
        idx3 = std::min(trackPosition.section+2, tubeSections.size()-1);

        P[0] = DirectX::XMLoadFloat3(&tubeSections[idx0]);
        P[3] = DirectX::XMLoadFloat3(&tubeSections[idx3]);

        // Update transform
        temp = DirectX::XMVectorCatmullRom(P[0], P[1], P[2], P[3], trackPosition.T);
        DirectX::XMStoreFloat3(&transform.position, temp);
    }
}
