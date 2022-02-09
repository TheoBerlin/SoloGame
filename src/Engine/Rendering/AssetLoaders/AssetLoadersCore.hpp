#pragma once

#include <Engine/Rendering/AssetLoaders/TextureCache.hpp>
#include <Engine/Rendering/AssetLoaders/ModelLoader.hpp>

class Device;
class RenderingCore;

class AssetLoadersCore
{
public:
    AssetLoadersCore(RenderingCore* pRenderingCore);
    ~AssetLoadersCore() = default;

    TextureCache* GetTextureCache() { return &m_TextureCache; }
    ModelLoader* GetModelLoader()   { return &m_ModelLoader; }

private:
    TextureCache m_TextureCache;
    ModelLoader m_ModelLoader;
};
