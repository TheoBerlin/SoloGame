#pragma once

#include <Engine/IGame.hpp>

class Game : public IGame
{
public:
    Game(HINSTANCE hInstance);
    ~Game();

    void update(float dt);

private:
    bool hasSetup;

    Entity camera, renderableObject;
};
