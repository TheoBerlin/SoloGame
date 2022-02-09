#include "TextureCache.hpp"

#include <Engine/Rendering/APIAbstractions/Device.hpp>
#include <Engine/Rendering/APIAbstractions/Texture.hpp>
#include <Engine/Utils/ECSUtils.hpp>

TextureCache::TextureCache(Device* pDevice)
    :m_pDevice(pDevice)
{}

std::shared_ptr<Texture> TextureCache::LoadTexture(const std::string& filePath)
{
    // See if the texture is already loaded
    auto itr = m_Textures.find(filePath);
    if (itr != m_Textures.end()) {
        std::weak_ptr<Texture>& texturePtr = itr->second;

        if (texturePtr.expired()) {
            // The texture used to exist but has been deleted
            m_Textures.erase(itr);
        } else {
            return texturePtr.lock();
        }
    }

    std::shared_ptr<Texture> texture(m_pDevice->createTextureFromFile(filePath));
    LOG_INFOF("Loaded texture: [%s]", filePath.c_str());

    m_Textures.insert({filePath, texture});

    return texture;
}
