#include "TextureLoader.hpp"

#include <WICTextureLoader/WICTextureLoader.h>
#include <Engine/Utils/Logger.hpp>

std::unordered_map<std::string, Texture> TextureLoader::textures = std::unordered_map<std::string, Texture>();

TextureLoader::~TextureLoader()
{
    this->deleteAllTextures();
}

Texture TextureLoader::loadTexture(const std::string& filePath)
{
    // See if the texture is already loaded
    auto itr = textures.find(filePath);

    if (itr != textures.end()) {
        // The texture exists
        return itr->second;
    }

    // Convert std::string to const wchar_t*
    const wchar_t* convertedFilePath = std::wstring(filePath.begin(), filePath.end()).c_str();

    // Load the texture
    Texture newTexture;

    ID3D11Resource* textureResource = nullptr;

    HRESULT hr = DirectX::CreateWICTextureFromFile(nullptr, convertedFilePath, &textureResource, &newTexture.srv);

    if (FAILED(hr)) {
        // Indicate a failure to load the texture
        newTexture.txType = TX_TYPE::NO_TEXTURE;

        Logger::LOG_WARNING("Failed to load texture: [%s]", filePath.c_str());
    }

    return Texture();
}

void TextureLoader::deleteAllTextures()
{
    for (auto texture : textures) {
        //texture.second.srv->Release();
    }
}
