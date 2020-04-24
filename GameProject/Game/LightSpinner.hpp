#pragma once

#include <Engine/ECS/System.hpp>

class LightHandler;

class LightSpinner : public System
{
public:
    LightSpinner(ECSCore* pECS);
    ~LightSpinner();

    virtual bool initSystem() override;

    void update(float dt);

private:
    LightHandler* m_pLightHandler;
    IDVector m_Lights;
};
