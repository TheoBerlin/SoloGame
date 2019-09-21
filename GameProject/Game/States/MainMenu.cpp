#include "MainMenu.hpp"

#include <Engine/ECS/ECSInterface.hpp>
#include <Engine/GameState/StateManager.hpp>
#include <Engine/InputHandler.hpp>
#include <Engine/Utils/Logger.hpp>
#include <Game/States/GameSession.hpp>

MainMenu::MainMenu(StateManager* stateManager, ECSInterface* ecs, ID3D11Device* device)
    :State(stateManager, ecs),
    device(device)
{
    std::type_index tid_inputHandler = std::type_index(typeid(InputHandler));

    InputHandler* inputHandler = static_cast<InputHandler*>(ecs->systemSubscriber.getComponentHandler(tid_inputHandler));
    this->keyboardState = inputHandler->getKeyboardState();

    Logger::LOG_INFO("Entered main menu, press E to start a game session");
}

MainMenu::~MainMenu()
{}

void MainMenu::resume()
{}

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
