#include "State.hpp"

State::State(StateManager* stateManager, ECSInterface* ecs)
    :ecs(ecs),
    stateManager(stateManager)
{}

State::~State()
{}

State::State(State* other)
    :ecs(other->ecs),
    stateManager(other->stateManager)
{}
