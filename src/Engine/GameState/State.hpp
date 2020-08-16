#pragma once

#include <Engine/GameState/StateManager.hpp>

class ECSCore;

class State
{
public:
    State(StateManager* pStateManager, ECSCore* pECS);
    State(State* pOther);
    virtual ~State() = 0 {};

    virtual void init() = 0;

    virtual void resume() = 0;
    virtual void pause() = 0;

    virtual void update(float dt) = 0;

protected:
    ECSCore* m_pECS;
    StateManager* m_pStateManager;
};
