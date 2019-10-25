#include "MainMenu.hpp"

#include <Engine/ECS/ECSInterface.hpp>
#include <Engine/GameState/StateManager.hpp>
#include <Engine/InputHandler.hpp>
#include <Engine/UI/Panel.hpp>
#include <Engine/Utils/Logger.hpp>
#include <Game/States/GameSession.hpp>

MainMenu::MainMenu(StateManager* stateManager, ECSInterface* ecs, ID3D11Device* device)
    :State(stateManager, ecs),
    device(device)
{
    std::type_index tid_inputHandler = std::type_index(typeid(InputHandler));

    this->inputHandler = static_cast<InputHandler*>(ecs->systemSubscriber.getComponentHandler(tid_inputHandler));
    inputHandler->setMouseMode(DirectX::Mouse::Mode::MODE_ABSOLUTE);
    inputHandler->setMouseVisibility(true);
    this->keyboardState = inputHandler->getKeyboardState();

    std::type_index tid_uiHandler = std::type_index(typeid(UIHandler));
    UIHandler* uiHandler = static_cast<UIHandler*>(ecs->systemSubscriber.getComponentHandler(tid_uiHandler));

    // Create UI panel
    uiEntity = ecs->entityIDGen.genID();

    uiHandler->createPanel(uiEntity, {0.4f, 0.45f}, {0.2f, 0.1f}, {1.0f, 0.3f, 0.3f, 1.0f}, "./Game/Assets/Models/Cube.png");

    // Make the panel a button
    uiHandler->createButton(uiEntity, {1.0f, 0.4f, 0.4f, 1.0f}, {0.9f, 0.2f, 0.2f, 1.0f}, [this](){this->stateManager->pushState(new GameSession(this));});

    Logger::LOG_INFO("Entered main menu, press E to start a game session");
}

MainMenu::~MainMenu()
{}

void MainMenu::resume()
{
    inputHandler->setMouseMode(DirectX::Mouse::Mode::MODE_ABSOLUTE);
    inputHandler->setMouseVisibility(true);
}

void MainMenu::pause()
{}

void MainMenu::update(float dt)
{
    if (keyboardState->E) {
        this->stateManager->pushState(new GameSession(this));
    }
}

ID3D11Device* MainMenu::getDevice()
{
    return device;
}
