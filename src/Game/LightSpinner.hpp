#pragma once

#include <Engine/ECS/System.hpp>

class TransformHandler;

class LightSpinner : public System
{
public:
    LightSpinner(ECSCore* pECS);
    ~LightSpinner();

    virtual bool initSystem() override;

    void update(float dt);

private:
    TransformHandler* m_pTransformHandler;
    IDVector m_Lights;
};
