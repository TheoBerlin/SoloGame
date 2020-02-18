#include "TextureLoader.hpp"

#include <Engine/Rendering/Display.hpp>
#include <Engine/Utils/Logger.hpp>

#include <DirectXTK/WICTextureLoader.h>

TextureLoader::TextureLoader(ECSCore* pECS, ID3D11Device* device)
    :device(device),
    ComponentHandler({}, pECS, std::type_index(typeid(TextureLoader)))
{}

TextureLoader::~TextureLoader()
{
    this->deleteAllTextures();
}

TextureReference TextureLoader::loadTexture(const std::string& filePath)
{
    // See if the texture is already loaded
    auto itr = textures.find(filePath);

    if (itr != textures.end()) {
        if (itr->second->getSRV() == nullptr) {
            // The texture was loaded but its data is now deleted, delete the texture pointer
            delete itr->second;
        } else {
            // The texture exists, create a reference to it
            return TextureReference(itr->second);
        }
    }

    // Convert std::string to const wchar_t*
    std::wstring wStrPath(filePath.begin(), filePath.end());
    const wchar_t* convertedFilePath = wStrPath.c_str();

    // Load the texture
    ID3D11ShaderResourceView* pSRV = nullptr;
    ID3D11Resource* textureResource = nullptr;

    HRESULT hr = DirectX::CreateWICTextureFromFile(TextureLoader::device, convertedFilePath, &textureResource, &pSRV);

    if (textureResource) {
        textureResource->Release();
    }

    if (FAILED(hr)) {
        Logger::LOG_WARNING("Failed to load texture: [%s]", filePath.c_str());
    } else {
        Logger::LOG_INFO("Loaded texture: [%s]", filePath.c_str());
        Texture* pTexture = new Texture(pSRV);
        textures[filePath] = pTexture;

        TextureReference textureReference(pTexture);
        return textureReference;
    }

    return TextureReference(nullptr);
}

void TextureLoader::deleteAllTextures()
{
    for (auto texturePair : textures) {
        delete texturePair.second;
    }

    textures.clear();
}
