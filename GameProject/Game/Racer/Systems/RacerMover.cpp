#include "RacerMover.hpp"

#include <Engine/Transform.hpp>
#include <Engine/Utils/DirectXUtils.hpp>
#include <Engine/Utils/ECSUtils.hpp>
#include <Game/Level/Tube.hpp>
#include <Game/Racer/Components/TrackPosition.hpp>

#include <cmath>

RacerMover::RacerMover(ECSCore* pECS)
    :System(pECS)
{
    TransformGroup transformSub;
    transformSub.m_Position.Permissions = RW;
    transformSub.m_Rotation.Permissions = RW;

    SystemRegistration sysReg = {
        {
            {{{RW, tid_trackPosition}}, {&transformSub}, &m_Racers, [this](Entity entity){racerAdded(entity);}}
        },
        this
    };

    subscribeToComponents(sysReg);
    registerUpdate(sysReg);
}

RacerMover::~RacerMover()
{}

bool RacerMover::init()
{
    m_pTransformHandler       = static_cast<TransformHandler*>(getComponentHandler(TID(TransformHandler)));
    m_pTrackPositionHandler   = static_cast<TrackPositionHandler*>(getComponentHandler(TID(TrackPositionHandler)));
    m_pTubeHandler            = static_cast<TubeHandler*>(getComponentHandler(TID(TubeHandler)));

    InputHandler* pInputHandler = static_cast<InputHandler*>(getComponentHandler(TID(InputHandler)));
    m_pKeyboardState = pInputHandler->getKeyboardState();

    return m_pTransformHandler && m_pTrackPositionHandler && m_pTubeHandler && pInputHandler;
}

void RacerMover::update(float dt)
{
    const std::vector<DirectX::XMFLOAT3>& tubeSections = m_pTubeHandler->getTubeSections();

    for (Entity entity : m_Racers.getIDs()) {
        Transform transform = m_pTransformHandler->getTransform(entity);
        TrackPosition& trackPosition = m_pTrackPositionHandler->trackPositions.indexID(entity);

        // Calculate the distance between P1 and P2 to figure out by how much to increase T per second
        DirectX::XMVECTOR P[4];
        P[1] = DirectX::XMLoadFloat3(&tubeSections[trackPosition.section]);
        P[2] = DirectX::XMLoadFloat3(&tubeSections[trackPosition.section + 1]);

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
            P[2] = DirectX::XMLoadFloat3(&tubeSections[trackPosition.section + 1]);

            temp = DirectX::XMVector3Length(DirectX::XMVectorSubtract(P[2], P[1]));
            float newSectionLength = DirectX::XMVectorGetX(temp);

            trackPosition.T = (newSectionLength / sectionLength) * (trackPosition.T - 1.0f);
            hasNotReachedEnd = (trackPosition.section != tubeSections.size() - 2) || (trackPosition.T < 1.0f);
        }

        // P[1] and P[2] are loaded since earlier
        size_t idx0, idx3;
        idx0 = std::max((int)trackPosition.section - 1, 0);
        idx3 = std::min(trackPosition.section + 2, tubeSections.size() - 1);

        P[0] = DirectX::XMLoadFloat3(&tubeSections[idx0]);
        P[3] = DirectX::XMLoadFloat3(&tubeSections[idx3]);

        // Update transform
        temp = DirectX::XMVectorCatmullRom(P[0], P[1], P[2], P[3], trackPosition.T);

        DirectX::XMVECTOR forward = DirectX::XMVector3Normalize(catmullRomDerivative(P[0], P[1], P[2], P[3], trackPosition.T));
        TransformHandler::setForward(transform.RotationQuaternion, forward);

        DirectX::XMVECTOR position = DirectX::XMVectorSubtract(temp, DirectX::XMVectorScale(TransformHandler::getUp(transform.RotationQuaternion), trackPosition.distanceFromCenter));
        DirectX::XMStoreFloat3(&transform.Position, position);

        // Roll using keyboard input
        int keyInput = m_pKeyboardState->D - m_pKeyboardState->A;
        if (keyInput) {
            float rotationAngle = keyInput * rotationSpeed * dt;
            TransformHandler::roll(transform.RotationQuaternion, rotationAngle);
        }

        // Move towards or away from the center
        keyInput = m_pKeyboardState->S - m_pKeyboardState->W;
        if (keyInput) {
            trackPosition.distanceFromCenter += keyInput * centerMoveSpeed * dt;
            trackPosition.distanceFromCenter = std::min(m_pTubeHandler->getTubeRadius() - minEdgeDistance, std::max(trackPosition.distanceFromCenter, minCenterDistance));
        }
    }
}

void RacerMover::racerAdded(Entity entity)
{
    Transform transform = m_pTransformHandler->getTransform(entity);

    // Calculate the position of the center point of the tube right at the beginning
    const std::vector<DirectX::XMFLOAT3>& tubeSections = m_pTubeHandler->getTubeSections();

    DirectX::XMVECTOR P01, P2, P3;
    P01 = DirectX::XMLoadFloat3(&tubeSections[0]);
    P2 = DirectX::XMLoadFloat3(&tubeSections[std::min((size_t)1, tubeSections.size() - 1)]);
    P3 = DirectX::XMLoadFloat3(&tubeSections[std::min((size_t)2, tubeSections.size() - 1)]);

    DirectX::XMVECTOR forward = DirectX::XMVector3Normalize(catmullRomDerivative(P01, P01, P2, P3, 0.001f));

    TransformHandler::setForward(transform.RotationQuaternion, forward);

    // Correct the roll
    DirectX::XMVECTOR tubeStartCenter = DirectX::XMVectorCatmullRom(P01, P01, P2, P3, 0.0f);

    DirectX::XMVECTOR desiredUp = DirectX::XMVector3Normalize(DirectX::XMVectorSubtract(tubeStartCenter, DirectX::XMLoadFloat3(&transform.Position)));
    DirectX::XMVECTOR currentUp = TransformHandler::getUp(transform.RotationQuaternion);

    float rollAngle = DirectX::XMVectorGetX(DirectX::XMVector3AngleBetweenNormals(desiredUp, currentUp));
    DirectX::XMVECTOR roll = DirectX::XMQuaternionRotationNormal(forward, rollAngle);

    DirectX::XMVECTOR currentRotQuat = DirectX::XMLoadFloat4(&transform.RotationQuaternion);
    DirectX::XMStoreFloat4(&transform.RotationQuaternion, DirectX::XMQuaternionMultiply(roll, currentRotQuat));

    // Set the distance to the tube's center
    TrackPosition& trackPosition = m_pTrackPositionHandler->trackPositions.indexID(entity);
    trackPosition.distanceFromCenter = minCenterDistance + startingCenterDistance * ((m_pTubeHandler->getTubeRadius() - minEdgeDistance) - minCenterDistance);

    // Set starting position
    DirectX::XMStoreFloat3(&transform.Position, DirectX::XMVectorSubtract(tubeStartCenter, DirectX::XMVectorScale(desiredUp, trackPosition.distanceFromCenter)));
}
