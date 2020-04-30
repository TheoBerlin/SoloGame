#pragma once

#include <Engine/Rendering/AssetLoaders/TextureLoader.hpp>
#include <Engine/Rendering/AssetLoaders/ModelLoader.hpp>

class ECSCore;
class IDevice;

class AssetLoadersCore
{
public:
    AssetLoadersCore(ECSCore* pECS, IDevice* pDevice);
    ~AssetLoadersCore();

private:
    TextureLoader m_TextureLoader;
    ModelLoader m_ModelLoader;
};
