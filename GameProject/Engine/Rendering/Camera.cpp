#include "Camera.hpp"

#include <Engine/Rendering/Components/VPMatrices.hpp>
#include <Engine/InputHandler.hpp>
#include <Engine/Transform.hpp>
#include <Engine/Utils/ECSUtils.hpp>

CameraSystem::CameraSystem(ECSCore* pECS)
    :System(pECS)
{
    SystemRegistration sysReg = {
    {
        {{{RW, tid_transform}, {RW, tid_view}, {R, tid_projection}}, &m_Cameras},
    },
    this};

    this->subscribeToComponents(sysReg);
    this->registerUpdate(sysReg);
}

CameraSystem::~CameraSystem()
{}

bool CameraSystem::init()
{
    this->m_pTransformHandler = static_cast<TransformHandler*>(getComponentHandler(TID(TransformHandler)));
    this->m_pVPHandler = static_cast<VPHandler*>(getComponentHandler(TID(VPHandler)));

    InputHandler* pInputHandler = static_cast<InputHandler*>(getComponentHandler(TID(InputHandler)));
    this->m_pKeyboardState = pInputHandler->getKeyboardState();
    this->m_pMouseState = pInputHandler->getMouseState();

    return m_pTransformHandler && m_pVPHandler && pInputHandler;
}

void CameraSystem::update(float dt)
{
    for (size_t i = 0; i < m_Cameras.size(); i += 1) {
        Transform& camTransform = m_pTransformHandler->transforms.indexID(m_Cameras[i]);
        ViewMatrix& viewMatrix = m_pVPHandler->viewMatrices.indexID(m_Cameras[i]);

        DirectX::XMVECTOR lookDir = m_pTransformHandler->getForward(camTransform.rotQuat);
        DirectX::XMVECTOR pitchAxis = DirectX::XMVector3Normalize(DirectX::XMVector3Cross(g_DefaultUp, lookDir));

        // React to mouse input
        if (m_pMouseState->x || m_pMouseState->y) {
            DirectX::XMVECTOR rotation = DirectX::XMLoadFloat4(&camTransform.rotQuat);

            // Limit pitch
            float pitch = m_pTransformHandler->getPitch(lookDir);
            float addedPitch = m_pMouseState->y * dt * 1.3f;
            float newPitch = pitch + addedPitch;

            if (std::abs(newPitch) > maxPitch) {
                addedPitch = newPitch > 0.0f ? maxPitch - pitch : -maxPitch - pitch;
            }

            rotation = DirectX::XMQuaternionMultiply(rotation, DirectX::XMQuaternionRotationAxis(g_DefaultUp, m_pMouseState->x * dt * 1.3f));
            rotation = DirectX::XMQuaternionMultiply(rotation, DirectX::XMQuaternionRotationAxis(pitchAxis, addedPitch));
            DirectX::XMStoreFloat4(&camTransform.rotQuat, rotation);
            lookDir = m_pTransformHandler->getForward(camTransform.rotQuat);
        }

        DirectX::XMVECTOR camPos = DirectX::XMLoadFloat3(&camTransform.position);

        // React to keyboard input
        if ((m_pKeyboardState->W-m_pKeyboardState->S) || (m_pKeyboardState->D-m_pKeyboardState->A) || (m_pKeyboardState->LeftShift-m_pKeyboardState->LeftControl)) {
            DirectX::XMVECTOR camMove = {0.0f, 0.0f, 0.0f, 0.0f};

            camMove = DirectX::XMVectorAdd(camMove, DirectX::XMVectorScale(lookDir, (float)(m_pKeyboardState->W-m_pKeyboardState->S)));
            camMove = DirectX::XMVectorAdd(camMove, DirectX::XMVectorScale(pitchAxis, (float)(m_pKeyboardState->D-m_pKeyboardState->A)));
            camMove = DirectX::XMVectorAdd(camMove, DirectX::XMVectorScale(g_DefaultUp, (float)(m_pKeyboardState->LeftShift-m_pKeyboardState->LeftControl)));

            camMove = DirectX::XMVectorScale(DirectX::XMVector3Normalize(camMove), dt * 1.5f);
            camPos = DirectX::XMVectorAdd(camPos, camMove);
            DirectX::XMStoreFloat3(&camTransform.position, camPos);
        }

        DirectX::XMVECTOR upDir = TransformHandler::getUp(camTransform.rotQuat);
        DirectX::XMStoreFloat4x4(&viewMatrix.view, DirectX::XMMatrixLookAtLH(camPos, DirectX::XMVectorAdd(camPos, lookDir), upDir));
    }
}
