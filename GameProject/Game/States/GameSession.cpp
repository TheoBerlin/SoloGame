#include "GameSession.hpp"

#include <Engine/ECS/ECSCore.hpp>
#include <Engine/Rendering/Components/Renderable.hpp>
#include <Engine/Rendering/Components/VPMatrices.hpp>
#include <Engine/Rendering/Renderer.hpp>
#include <Engine/Transform.hpp>
#include <Engine/Utils/Logger.hpp>
#include <Game/States/MainMenu.hpp>

GameSession::GameSession(MainMenu* mainMenu)
    :State(mainMenu, STATE_TRANSITION::POP_AND_PUSH),
    tubeHandler(m_pECS, mainMenu->getDevice()),
    trackPositionHandler(m_pECS),
    lightSpinner(m_pECS),
    racerMover(m_pECS)
{
    Logger::LOG_INFO("Started game session");

    // Set mouse mode to relative, which also hides the mouse
    const std::type_index tid_inputHandler = std::type_index(typeid(InputHandler));

    this->inputHandler = static_cast<InputHandler*>(m_pECS->getSystemSubscriber()->getComponentHandler(tid_inputHandler));
    inputHandler->setMouseMode(DirectX::Mouse::Mode::MODE_RELATIVE);
    inputHandler->setMouseVisibility(false);

    // Create camera
    camera = m_pECS->createEntity();

    const std::type_index tid_transformHandler = std::type_index(typeid(TransformHandler));
    TransformHandler* transformHandler = static_cast<TransformHandler*>(m_pECS->getSystemSubscriber()->getComponentHandler(tid_transformHandler));

    transformHandler->createTransform(camera);
    Transform& camTransform = transformHandler->transforms.indexID(camera);
    camTransform.position = {2.0f, 1.8f, 3.3f};

    const std::type_index tid_vpHandler = std::type_index(typeid(VPHandler));
    VPHandler* vpHandler = static_cast<VPHandler*>(m_pECS->getSystemSubscriber()->getComponentHandler(tid_vpHandler));

    DirectX::XMVECTOR camPos = DirectX::XMLoadFloat3(&camTransform.position);
    DirectX::XMVECTOR camLookDir = transformHandler->getForward(camTransform.rotQuat);
    DirectX::XMVECTOR camUpDir = {0.0f, 1.0f, 0.0f, 0.0f};
    vpHandler->createViewMatrix(camera, camPos, camLookDir, camUpDir);
    vpHandler->createProjMatrix(camera, 90.0f, 16.0f/9.0f, 0.1f, 20.0f);

    // Create renderable object
    std::type_index tid_renderableHandler = std::type_index(typeid(RenderableHandler));
    RenderableHandler* renderableHandler = static_cast<RenderableHandler*>(m_pECS->getSystemSubscriber()->getComponentHandler(tid_renderableHandler));

    renderableObject = m_pECS->createEntity();
    transformHandler->createTransform(renderableObject);
    transformHandler->createWorldMatrix(renderableObject);
    renderableHandler->createRenderable(renderableObject, "./Game/Assets/Models/Cube.dae", PROGRAM::BASIC);

    // Create point lights
    std::type_index tid_lightHandler = std::type_index(typeid(LightHandler));
    LightHandler* lightHandler = static_cast<LightHandler*>(m_pECS->getSystemSubscriber()->getComponentHandler(tid_lightHandler));

    for (unsigned i = 0; i < MAX_POINTLIGHTS; i += 1) {
        Entity lightID = m_pECS->createEntity();
        DirectX::XMFLOAT3 lightPos = {std::sinf(DirectX::XM_PIDIV2 * i) * 3.0f, 1.0f, std::cosf(DirectX::XM_PIDIV2 * i) * 3.0f};
        DirectX::XMFLOAT3 light = {std::sinf(1.6f * i), 0.8f, std::cosf(1.2f * i)};

        lightHandler->createPointLight(lightID, lightPos, light, 10.0f);
    }

    // Create tube
    std::vector<DirectX::XMFLOAT3> sectionPoints = {
            {0.0f, 0.0f, 0.0f},
            {4.0f, 4.0f, -10.0f},
            {2.0f, 2.0f, -22.0f},
            {0.0f, 0.0f, -32.0f},
            {-3.0f, -6.0f, -42.0f},
    };

    const float tubeRadius = 1.5f;
    const unsigned int tubeFaces = 10;
    Model* tubeModel = tubeHandler.createTube(sectionPoints, tubeRadius, tubeFaces);

    Entity tube = m_pECS->createEntity();
    renderableHandler->createRenderable(tube, tubeModel, PROGRAM::BASIC);
    transformHandler->createTransform(tube);
    transformHandler->createWorldMatrix(tube);

    trackPositionHandler.createTrackPosition(camera);
}

GameSession::~GameSession()
{}

void GameSession::resume()
{
    inputHandler->setMouseMode(DirectX::Mouse::Mode::MODE_RELATIVE);
    inputHandler->setMouseVisibility(false);
}

void GameSession::pause()
{}

void GameSession::update(float dt)
{
    m_pECS->getSystemUpdater()->updateMT(dt);
}
