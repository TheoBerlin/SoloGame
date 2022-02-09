#include "AssetLoadersCore.hpp"

AssetLoadersCore::AssetLoadersCore(RenderingCore* pRenderingCore)
    :   m_TextureCache(pRenderingCore->GetDevice())
    ,   m_ModelLoader(&m_TextureCache, pRenderingCore->GetDevice())
{}
