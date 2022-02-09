#pragma once

#include <Engine/ECS/System.hpp>

class LightSpinner : System
{
public:
    LightSpinner();
    ~LightSpinner() = default;

    void Update(float dt);

private:
    IDVector m_Lights;
};
