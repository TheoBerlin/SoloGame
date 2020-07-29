#include "TextureCache.hpp"

#include <Engine/Rendering/APIAbstractions/Device.hpp>
#include <Engine/Rendering/APIAbstractions/Texture.hpp>
#include <Engine/Utils/ECSUtils.hpp>
#include <Engine/Utils/Logger.hpp>

TextureCache::TextureCache(ECSCore* pECS, Device* pDevice)
    :ComponentHandler(pECS, TID(TextureCache)),
    m_pDevice(pDevice)
{
    ComponentHandlerRegistration handlerReg = {};
    handlerReg.pComponentHandler = this;
    registerHandler(handlerReg);
}

bool TextureCache::initHandler()
{
    return true;
}

std::shared_ptr<Texture> TextureCache::loadTexture(const std::string& filePath)
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
