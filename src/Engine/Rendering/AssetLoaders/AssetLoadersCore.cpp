#include "AssetLoadersCore.hpp"

AssetLoadersCore::AssetLoadersCore(ECSCore* pECS, Device* pDevice)
    :m_TextureLoader(pECS, pDevice),
    m_ModelLoader(pECS, &m_TextureLoader, pDevice)
{}
