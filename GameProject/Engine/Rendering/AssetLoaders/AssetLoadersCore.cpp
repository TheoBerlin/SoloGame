#include "AssetLoadersCore.hpp"

AssetLoadersCore::AssetLoadersCore(ECSCore* pECS, IDevice* pDevice)
    :m_TextureLoader(pECS, pDevice),
    m_ModelLoader(pECS, &m_TextureLoader, reinterpret_cast<DeviceDX11*>(pDevice))
{}

AssetLoadersCore::~AssetLoadersCore()
{}
