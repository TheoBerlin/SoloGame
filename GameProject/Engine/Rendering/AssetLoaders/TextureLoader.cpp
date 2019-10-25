#include "TextureLoader.hpp"

#include <DirectXTK/WICTextureLoader.h>
#include <Engine/Rendering/Display.hpp>
#include <Engine/Utils/Logger.hpp>

TextureLoader::TextureLoader(SystemSubscriber* sysSubscriber, ID3D11Device* device)
    :device(device),
    ComponentHandler({}, sysSubscriber, std::type_index(typeid(TextureLoader)))
{}

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
    std::wstring wStrPath(filePath.begin(), filePath.end());
    const wchar_t* convertedFilePath = wStrPath.c_str();

    // Load the texture
    Texture texture;
    ID3D11Resource* textureResource = nullptr;

    HRESULT hr = DirectX::CreateWICTextureFromFile(TextureLoader::device, convertedFilePath, &textureResource, &texture.srv);

    if (textureResource) {
        textureResource->Release();
    }

    if (FAILED(hr)) {
        Logger::LOG_WARNING("Failed to load texture: [%s]", filePath.c_str());
    } else {
        Logger::LOG_INFO("Loaded texture: [%s]", filePath.c_str());
        textures[filePath] = texture;
    }

    return texture;
}

void TextureLoader::deleteAllTextures()
{
    for (auto texture : textures) {
        texture.second.srv->Release();
    }

    textures.clear();
}
