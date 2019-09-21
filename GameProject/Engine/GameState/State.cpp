#include "State.hpp"

State::State(StateManager* stateManager, ECSInterface* ecs)
    :ecs(ecs),
    stateManager(stateManager)
{}
