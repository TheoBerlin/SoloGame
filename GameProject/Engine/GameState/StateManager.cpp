#include "StateManager.hpp"

#include <Engine/GameState/State.hpp>

StateManager::StateManager()
{}

StateManager::~StateManager()
{
    while (states.size() != 0) {
        delete states.top();
        states.pop();
    }
}

void StateManager::pushState(State* state)
{
    states.top()->pause();

    states.push(state);
}

void StateManager::popState()
{
    delete states.top();
    states.pop();

    states.top()->resume();
}

void StateManager::update(float dt)
{
    states.top()->update(dt);
}
