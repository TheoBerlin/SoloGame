#pragma once

#include <Engine/ECS/ECSCore.hpp>

#include <d3d11.h>

class Renderer
{
public:
    Renderer(ECSCore* pECS);
    ~Renderer();

    bool init(ID3D11Device* pDevice);

    virtual void recordCommands() = 0;

    void beginFrame();
    void endFrame();
    void executeCommands();

private:
    ECSCore* m_pECS;

    // Deferred context for recording commands
    ID3D11DeviceContext* m_pDeferredContext;
};
