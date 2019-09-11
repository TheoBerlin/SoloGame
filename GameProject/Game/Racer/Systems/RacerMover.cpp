#include "RacerMover.hpp"

#include <Engine/ECS/ECSInterface.hpp>
#include <Engine/Transform.hpp>
#include <Engine/Utils/DirectXUtils.hpp>
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
        {{{RW, tid_transform}, {RW, tid_trackPosition}}, &racers, [this](Entity entity){onRacerAdded(entity);}}
    },
    this};

    this->subscribeToComponents(&sysReg);
    this->registerUpdate(&sysReg);

    std::type_index tid_inputHandler = std::type_index(typeid(InputHandler));
    InputHandler* inputHandler = static_cast<InputHandler*>(ecs->systemSubscriber.getComponentHandler(tid_inputHandler));
    this->keyboardState = inputHandler->getKeyboardState();
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

        DirectX::XMVECTOR forward = DirectX::XMVector3Normalize(catmullRomDerivative(P[0], P[1], P[2], P[3], trackPosition.T));
        TransformHandler::setForward(transform, forward);

        DirectX::XMStoreFloat3(&transform.position, DirectX::XMVectorSubtract(temp, TransformHandler::getUp(transform.rotQuat)));

        // Roll using keyboard input
        if (keyboardState->D - keyboardState->A) {
            float rotationAngle = (keyboardState->D - keyboardState->A) * rotationSpeed * dt;
            TransformHandler::roll(transform.rotQuat, rotationAngle);
        }
    }
}

void RacerMover::onRacerAdded(Entity entity)
{
    Transform& transform = transformHandler->transforms.indexID(entity);

    // Calculate the position of the center point of the tube right at the beginning
    const std::vector<DirectX::XMFLOAT3>& tubeSections = tubeHandler->getTubeSections();

    DirectX::XMVECTOR P01, P2, P3;
    P01 = DirectX::XMLoadFloat3(&tubeSections[0]);
    P2 = DirectX::XMLoadFloat3(&tubeSections[std::min((size_t)1, tubeSections.size()-1)]);
    P3 = DirectX::XMLoadFloat3(&tubeSections[std::min((size_t)2, tubeSections.size()-1)]);

    DirectX::XMVECTOR f = DirectX::XMVector3Normalize(catmullRomDerivative(P01, P01, P2, P3, 0.001f));

    TransformHandler::setForward(transform, f);

    // Correct the roll
    DirectX::XMVECTOR tubeStartCenter = DirectX::XMVectorCatmullRom(P01, P01, P2, P3, 0.0f);

    DirectX::XMVECTOR desiredUp = DirectX::XMVector3Normalize(DirectX::XMVectorSubtract(tubeStartCenter, DirectX::XMLoadFloat3(&transform.position)));
    DirectX::XMVECTOR currentUp = TransformHandler::getUp(transform.rotQuat);

    //float rollAngle = std::acosf(DirectX::XMVectorGetX(DirectX::XMVector3Dot(desiredUp, currentUp)));
    float rollAngle = DirectX::XMVectorGetX(DirectX::XMVector3AngleBetweenNormals(desiredUp, currentUp));
    DirectX::XMVECTOR roll = DirectX::XMQuaternionRotationNormal(f, rollAngle);

    DirectX::XMVECTOR currentRotQuat = DirectX::XMLoadFloat4(&transform.rotQuat);
    DirectX::XMStoreFloat4(&transform.rotQuat, DirectX::XMQuaternionMultiply(roll, currentRotQuat));
}
