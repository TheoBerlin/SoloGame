#pragma once

#include <stack>
#include <queue>
#include <vector>

// What to do when pushing a new state
enum class STATE_TRANSITION {
    PUSH,
    POP,
    PAUSE_AND_PUSH,
    POP_AND_PUSH
};

class ECSCore;
class State;

class StateManager
{
public:
    StateManager(ECSCore* pECS);
    ~StateManager();

    void transitionState(State* pNewState, STATE_TRANSITION transitionSetting);

    void update(float dt);

private:
    // Stack of game states with vector as storage class to allow for contiguous element storage
    std::stack<State*, std::vector<State*>> m_States;
    // Old states can't be deleted during transitions because their resources might be needed for the new states. They are instead enqueued for deletion.
    std::queue<State*> m_StatesToDelete;
    ECSCore* m_pECS;
};
