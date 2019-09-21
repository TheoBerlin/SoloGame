#pragma once

class ECSInterface;
class StateManager;

class State
{
public:
    State(StateManager* stateManager, ECSInterface* ecs);
    State(State* other);

    virtual void resume() = 0;
    virtual void pause() = 0;

    virtual void update(float dt) = 0;

protected:
    ECSInterface* ecs;
    StateManager* stateManager;
};
