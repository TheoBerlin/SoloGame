#pragma once

#include <stack>
#include <vector>

class State;

class StateManager
{
public:
    StateManager();
    ~StateManager();

    void pushState(State* state);
    void popState();

    void update(float dt);

private:
    // Stack of game states with vector as storage class to allow for contiguous element storage
    std::stack<State*, std::vector<State*>> states;
};
