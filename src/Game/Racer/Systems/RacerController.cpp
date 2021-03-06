#include "RacerController.hpp"

#include <Engine/InputHandler.hpp>
#include <Engine/Physics/Velocity.hpp>
#include <Engine/Transform.hpp>
#include <Engine/Utils/DirectXUtils.hpp>
#include <Engine/Utils/ECSUtils.hpp>
#include <Game/Level/Tube.hpp>
#include <Game/Racer/Components/Track.hpp>

#include <cmath>

RacerController::RacerController(ECSCore* pECS, InputHandler* pInputHandler, TubeHandler* pTubeHandler)
    :System(pECS),
    m_pInputHandler(pInputHandler),
    m_pTransformHandler(nullptr),
    m_pTrackHandler(nullptr),
    m_pTubeHandler(pTubeHandler),
    m_pVelocityHandler(nullptr)
{
    SystemRegistration sysReg = {};
    sysReg.SubscriberRegistration.EntitySubscriptionRegistrations = {
        {{{RW, g_TIDPosition}, {RW, g_TIDRotation}, {RW, g_TIDTrackPosition}, {RW, g_TIDTrackSpeed}, {RW, g_TIDVelocity}}, &m_Racers, [this](Entity entity){ racerAdded(entity); }}
    };
    sysReg.Phase = 1u;

    enqueueRegistration(sysReg);
}

bool RacerController::initSystem()
{
    m_pTransformHandler = reinterpret_cast<TransformHandler*>(getComponentHandler(TID(TransformHandler)));
    m_pTrackHandler     = reinterpret_cast<TrackHandler*>(getComponentHandler(TID(TrackHandler)));
    m_pVelocityHandler  = reinterpret_cast<VelocityHandler*>(getComponentHandler(TID(VelocityHandler)));

    return m_pTransformHandler && m_pTrackHandler && m_pVelocityHandler;
}

void RacerController::update(float dt)
{
    const std::vector<DirectX::XMFLOAT3>& tubeSections = m_pTubeHandler->getTubeSections();

    for (Entity entity : m_Racers.getIDs()) {
        DirectX::XMFLOAT4& rotationQuat = m_pTransformHandler->getRotation(entity);
        TrackPosition& trackPosition = m_pTrackHandler->getTrackPosition(entity);
        float& racerSpeed = m_pTrackHandler->getTrackSpeed(entity);

        // Accelerate or deccelerate using keyboard input
        int keyInput = m_pInputHandler->keyState(GLFW_KEY_LEFT_SHIFT) - m_pInputHandler->keyState(GLFW_KEY_LEFT_CONTROL);
        if (keyInput) {
            racerSpeed = std::clamp(racerSpeed + keyInput * g_RacerAcceleration * dt, g_RacerMinSpeed, g_RacerMaxSpeed);
        }

        // Roll using keyboard input
        keyInput = m_pInputHandler->keyState(GLFW_KEY_D) - m_pInputHandler->keyState(GLFW_KEY_A);
        if (keyInput) {
            float rotationAngle = keyInput * rotationSpeed * dt;
            TransformHandler::roll(rotationQuat, rotationAngle);
        }

        // Move towards or away from the center
        keyInput = m_pInputHandler->keyState(GLFW_KEY_S) - m_pInputHandler->keyState(GLFW_KEY_W);
        if (keyInput) {
            trackPosition.distanceFromCenter += keyInput * centerMoveSpeed * dt;
            trackPosition.distanceFromCenter = std::min(m_pTubeHandler->getTubeRadius() - minEdgeDistance, std::max(trackPosition.distanceFromCenter, minCenterDistance));
        }

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
        DirectX::XMVECTOR newCurvePosition = DirectX::XMVectorCatmullRom(P[0], P[1], P[2], P[3], trackPosition.T);

        DirectX::XMVECTOR forward = DirectX::XMVector3Normalize(catmullRomDerivative(P[0], P[1], P[2], P[3], trackPosition.T));
        TransformHandler::setForward(rotationQuat, forward);

        DirectX::XMVECTOR newPosition = DirectX::XMVectorSubtract(newCurvePosition, DirectX::XMVectorScale(TransformHandler::getUp(rotationQuat), trackPosition.distanceFromCenter));
        DirectX::XMVECTOR oldPosition = DirectX::XMLoadFloat3(&m_pTransformHandler->getPosition(entity));

        DirectX::XMVECTOR velocity = DirectX::XMVectorSubtract(newPosition, oldPosition);
        DirectX::XMStoreFloat3(&m_pVelocityHandler->getVelocity(entity), velocity);
    }
}

void RacerController::racerAdded(Entity entity)
{
    DirectX::XMFLOAT3& transformPosition    = m_pTransformHandler->getPosition(entity);
    DirectX::XMFLOAT4& rotationQuat         = m_pTransformHandler->getRotation(entity);

    // Calculate the position of the center point of the tube right at the beginning
    const std::vector<DirectX::XMFLOAT3>& tubeSections = m_pTubeHandler->getTubeSections();

    DirectX::XMVECTOR P01, P2, P3;
    P01 = DirectX::XMLoadFloat3(&tubeSections[0]);
    P2 = DirectX::XMLoadFloat3(&tubeSections[std::min((size_t)1, tubeSections.size() - 1)]);
    P3 = DirectX::XMLoadFloat3(&tubeSections[std::min((size_t)2, tubeSections.size() - 1)]);

    DirectX::XMVECTOR forward = DirectX::XMVector3Normalize(catmullRomDerivative(P01, P01, P2, P3, 0.001f));

    TransformHandler::setForward(rotationQuat, forward);

    // Correct the roll
    DirectX::XMVECTOR tubeStartCenter = DirectX::XMVectorCatmullRom(P01, P01, P2, P3, 0.0f);

    DirectX::XMVECTOR desiredUp = DirectX::XMVector3Normalize(DirectX::XMVectorSubtract(tubeStartCenter, DirectX::XMLoadFloat3(&transformPosition)));
    DirectX::XMVECTOR currentUp = TransformHandler::getUp(rotationQuat);

    float rollAngle = DirectX::XMVectorGetX(DirectX::XMVector3AngleBetweenNormals(desiredUp, currentUp));
    DirectX::XMVECTOR roll = DirectX::XMQuaternionRotationNormal(forward, rollAngle);

    DirectX::XMVECTOR currentRotQuat = DirectX::XMLoadFloat4(&rotationQuat);
    DirectX::XMStoreFloat4(&rotationQuat, DirectX::XMQuaternionMultiply(roll, currentRotQuat));

    // Set the distance to the tube's center
    TrackPosition& trackPosition = m_pTrackHandler->getTrackPosition(entity);
    trackPosition.distanceFromCenter = minCenterDistance + startingCenterDistance * ((m_pTubeHandler->getTubeRadius() - minEdgeDistance) - minCenterDistance);

    // Set starting position
    DirectX::XMStoreFloat3(&transformPosition, DirectX::XMVectorSubtract(tubeStartCenter, DirectX::XMVectorScale(desiredUp, trackPosition.distanceFromCenter)));

    // Set starting speed
    float& racerSpeed = m_pTrackHandler->getTrackSpeed(entity);
    racerSpeed = g_RacerMinSpeed;
}
