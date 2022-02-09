#pragma once

#include <stack>
#include <vector>
#include <queue>

class State;

// What to do when pushing a new state
enum class STATE_TRANSITION {
    PUSH,
    POP,
    PAUSE_AND_PUSH,
    POP_AND_PUSH
};

class StateManager
{
public:
    StateManager();
    ~StateManager();

    void Release();

    // The enqueued transition happens at the end of StateManager::Update
    void EnqueueStateTransition(State* pNewState, STATE_TRANSITION transitionSetting);
    void EnqueueStatePop() { EnqueueStateTransition(nullptr, STATE_TRANSITION::POP); }

    void Update(float dt);

private:
    void TransitionState();

    // Stack of game states with vector as storage class to allow for contiguous element storage
    std::stack<State*, std::vector<State*>> m_States;
    // Old states can't be deleted during transitions because their resources might be needed by the new states. They are instead enqueued for deletion.
    std::queue<State*> m_StatesToDelete;

    State* m_pEnqueuedState;
    STATE_TRANSITION m_EnqueuedTransitionAction;
};
