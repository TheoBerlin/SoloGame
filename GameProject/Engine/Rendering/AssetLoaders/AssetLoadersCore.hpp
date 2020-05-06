#pragma once

#include <Engine/Rendering/AssetLoaders/TextureCache.hpp>
#include <Engine/Rendering/AssetLoaders/ModelLoader.hpp>

class ECSCore;
class Device;

class AssetLoadersCore
{
public:
    AssetLoadersCore(ECSCore* pECS, Device* pDevice);
    ~AssetLoadersCore();

private:
    TextureCache m_TextureLoader;
    ModelLoader m_ModelLoader;
};
