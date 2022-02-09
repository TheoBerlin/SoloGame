#pragma once

class PhysicsCore;
class AssetLoadersCore;
class UICore;
class RenderingCore;
class AudioCore;

struct EngineConfig;

class EngineCore
{
public:
    EngineCore();
    ~EngineCore();

    bool Init();

    PhysicsCore* GetPhysicsCore()           { return m_pPhysicsCore; }
    AssetLoadersCore* GetAssetLoadersCore() { return m_pAssetLoaders; }
    UICore* GetUICore()                     { return m_pUICore; }
    RenderingCore* GetRenderingCore()       { return m_pRenderingCore; }
    AudioCore* GetAudioCore()               { return m_pAudioCore; }

public:
    static void SetInstance(EngineCore* pInstance)  { s_pInstance = pInstance; }
    static EngineCore* GetInstance()                { return s_pInstance; }

private:
    static EngineCore* s_pInstance;

private:
    bool LoadEngineConfig(EngineConfig& engineConfig) const;

private:
    PhysicsCore* m_pPhysicsCore;
    AssetLoadersCore* m_pAssetLoaders;
    UICore* m_pUICore;
    RenderingCore* m_pRenderingCore;
    AudioCore* m_pAudioCore;
};
