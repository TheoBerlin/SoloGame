#pragma once

#include <Engine/ECS/System.hpp>

class LightHandler;

class LightSpinner : public System
{
public:
    LightSpinner(ECSCore* pECS);
    ~LightSpinner();

    void update(float dt);

private:
    LightHandler* lightHandler;
    IDVector<Entity> lights;
};
