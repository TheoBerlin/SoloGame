#include "RacerController.hpp"

#include <Engine/InputHandler.hpp>
#include <Engine/Physics/Velocity.hpp>
#include <Engine/Transform.hpp>
#include <Engine/Utils/DirectXUtils.hpp>
#include <Engine/Utils/ECSUtils.hpp>
#include <Game/Level/Tube.hpp>
#include <Game/Racer/Components/Track.hpp>

#include <cmath>

RacerController::RacerController(TubeHandler* pTubeHandler)
    :m_pTubeHandler(pTubeHandler)
{
    SystemRegistration sysReg = {};
    sysReg.SubscriberRegistration.EntitySubscriptionRegistrations = {
        {
            .pSubscriber = &m_Racers,
            .ComponentAccesses =
            {
                { RW, PositionComponent::Type() }, { RW, RotationComponent::Type() }, { RW, TrackPositionComponent::Type() },
                { RW, TrackSpeedComponent::Type() }, { RW, VelocityComponent::Type() },
            },
            .OnEntityAdded = std::bind_front(&RacerController::OnRacerAdded, this)
        }
    };
    sysReg.Phase = 1u;

    RegisterSystem(TYPE_NAME(RacerController), sysReg);
}

void RacerController::Update(float dt)
{
    const std::vector<DirectX::XMFLOAT3>& tubeSections = m_pTubeHandler->GetTubeSections();

    ECSCore* pECS = ECSCore::GetInstance();
    ComponentArray<PositionComponent>* pPositionComponents = pECS->GetComponentArray<PositionComponent>();
    ComponentArray<RotationComponent>* pRotationComponents = pECS->GetComponentArray<RotationComponent>();
    ComponentArray<VelocityComponent>* pVelocityComponents = pECS->GetComponentArray<VelocityComponent>();
    ComponentArray<TrackPositionComponent>* pTrackPositionComponents = pECS->GetComponentArray<TrackPositionComponent>();
    ComponentArray<TrackSpeedComponent>* pTrackSpeedComponents = pECS->GetComponentArray<TrackSpeedComponent>();

    for (Entity entity : m_Racers) {
        DirectX::XMFLOAT4& rotationQuat = pRotationComponents->GetData(entity).Quaternion;
        TrackPositionComponent& trackPosition =  pTrackPositionComponents->GetData(entity);
        float& racerSpeed = pTrackSpeedComponents->GetData(entity).Speed;

        // Accelerate or deccelerate using keyboard input
        InputHandler* pInputHandler = EngineCore::GetInstance()->GetRenderingCore()->GetWindow()->GetInputHandler();
        int keyInput = pInputHandler->KeyState(GLFW_KEY_LEFT_SHIFT) - pInputHandler->KeyState(GLFW_KEY_LEFT_CONTROL);
        if (keyInput) {
            racerSpeed = std::clamp(racerSpeed + keyInput * g_RacerAcceleration * dt, g_RacerMinSpeed, g_RacerMaxSpeed);
        }

        // Roll using keyboard input
        keyInput = pInputHandler->KeyState(GLFW_KEY_D) - pInputHandler->KeyState(GLFW_KEY_A);
        if (keyInput) {
            float rotationAngle = keyInput * g_RotationSpeed * dt;
            Roll(rotationQuat, rotationAngle);
        }

        // Move towards or away from the center
        keyInput = pInputHandler->KeyState(GLFW_KEY_S) - pInputHandler->KeyState(GLFW_KEY_W);
        if (keyInput) {
            trackPosition.distanceFromCenter += keyInput * g_CenterMoveSpeed * dt;
            trackPosition.distanceFromCenter = std::min(m_pTubeHandler->GetTubeRadius() - g_MinEdgeDistance, std::max(trackPosition.distanceFromCenter, g_MinCenterDistance));
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
            const float newSectionLength = DirectX::XMVectorGetX(temp);

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

        DirectX::XMVECTOR forward = DirectX::XMVector3Normalize(CatmullRomDerivative(P[0], P[1], P[2], P[3], trackPosition.T));
        SetForward(rotationQuat, forward);

        DirectX::XMVECTOR newPosition = DirectX::XMVectorSubtract(newCurvePosition, DirectX::XMVectorScale(GetUp(rotationQuat), trackPosition.distanceFromCenter));
        DirectX::XMVECTOR oldPosition = DirectX::XMLoadFloat3(&pPositionComponents->GetData(entity).Position);

        DirectX::XMVECTOR velocity = DirectX::XMVectorSubtract(newPosition, oldPosition);
        DirectX::XMStoreFloat3(&pVelocityComponents->GetData(entity).Velocity, velocity);
    }
}

void RacerController::OnRacerAdded(Entity entity)
{
    ECSCore* pECS = ECSCore::GetInstance();
    DirectX::XMFLOAT3& transformPosition    = pECS->GetComponent<PositionComponent>(entity).Position;
    DirectX::XMFLOAT4& rotationQuat         = pECS->GetComponent<RotationComponent>(entity).Quaternion;

    // Calculate the position of the center point of the tube right at the beginning
    const std::vector<DirectX::XMFLOAT3>& tubeSections = m_pTubeHandler->GetTubeSections();

    DirectX::XMVECTOR P01, P2, P3;
    P01 = DirectX::XMLoadFloat3(&tubeSections[0]);
    P2 = DirectX::XMLoadFloat3(&tubeSections[std::min(1ull, tubeSections.size() - 1)]);
    P3 = DirectX::XMLoadFloat3(&tubeSections[std::min(2ull, tubeSections.size() - 1)]);

    DirectX::XMVECTOR forward = DirectX::XMVector3Normalize(CatmullRomDerivative(P01, P01, P2, P3, 0.001f));

    SetForward(rotationQuat, forward);

    // Correct the roll
    DirectX::XMVECTOR tubeStartCenter = DirectX::XMVectorCatmullRom(P01, P01, P2, P3, 0.0f);

    DirectX::XMVECTOR desiredUp = DirectX::XMVector3Normalize(DirectX::XMVectorSubtract(tubeStartCenter, DirectX::XMLoadFloat3(&transformPosition)));
    DirectX::XMVECTOR currentUp = GetUp(rotationQuat);

    float rollAngle = DirectX::XMVectorGetX(DirectX::XMVector3AngleBetweenNormals(desiredUp, currentUp));
    DirectX::XMVECTOR roll = DirectX::XMQuaternionRotationNormal(forward, rollAngle);

    DirectX::XMVECTOR currentRotQuat = DirectX::XMLoadFloat4(&rotationQuat);
    DirectX::XMStoreFloat4(&rotationQuat, DirectX::XMQuaternionMultiply(roll, currentRotQuat));

    // Set the distance to the tube's center
    TrackPositionComponent& trackPosition = pECS->GetComponent<TrackPositionComponent>(entity);
    trackPosition.distanceFromCenter = g_MinCenterDistance + g_StartingCenterDistance * ((m_pTubeHandler->GetTubeRadius() - g_MinEdgeDistance) - g_MinCenterDistance);

    // Set starting position
    DirectX::XMStoreFloat3(&transformPosition, DirectX::XMVectorSubtract(tubeStartCenter, DirectX::XMVectorScale(desiredUp, trackPosition.distanceFromCenter)));

    // Set starting speed
    pECS->GetComponent<TrackSpeedComponent>(entity).Speed = g_RacerMinSpeed;
}
