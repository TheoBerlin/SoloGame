#include "AssetLoadersCore.hpp"

AssetLoadersCore::AssetLoadersCore(ECSCore* pECS, ID3D11Device* pDevice)
    :m_TextureLoader(pECS, pDevice),
    m_ModelLoader(pECS, &m_TextureLoader)
{}

AssetLoadersCore::~AssetLoadersCore()
{}
