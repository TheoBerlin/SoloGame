#include "GameSession.hpp"

#include <Engine/ECS/ECSInterface.hpp>
#include <Game/States/MainMenu.hpp>

GameSession::GameSession(MainMenu* mainMenu)
    :State(mainMenu),
    tubeHandler(&ecs->systemSubscriber, mainMenu->getDevice()),
    trackPositionHandler(&ecs->systemSubscriber),
    lightSpinner(ecs),
    racerMover(ecs)
{}

GameSession::~GameSession()
{
    // TODO: Systems in the main menu might've registered some of the entities created by the GameSession. Make sure these are unregistered.
    ecs->systemUpdater.deregisterSystem(&lightSpinner);
    ecs->systemUpdater.deregisterSystem(&racerMover);
}

void GameSession::resume()
{}

void GameSession::pause()
{}

void GameSession::update(float dt)
{
    ecs->systemUpdater.updateMT(dt);
}
