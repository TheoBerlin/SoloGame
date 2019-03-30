#include "TextureLoader.hpp"

#include <WICTextureLoader/WICTextureLoader.h>
#include <Engine/Rendering/Display.hpp>
#include <Engine/Utils/Logger.hpp>

std::unordered_map<std::string, ID3D11ShaderResourceView*> TextureLoader::textures = std::unordered_map<std::string, ID3D11ShaderResourceView*>();
ID3D11Device* TextureLoader::device = nullptr;

TextureLoader::~TextureLoader()
{
    this->deleteAllTextures();
}

void TextureLoader::setDevice(ID3D11Device* device)
{
    TextureLoader::device = device;
}

ID3D11ShaderResourceView* TextureLoader::loadTexture(const std::string& filePath)
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
    ID3D11Resource* textureResource = nullptr;
    ID3D11ShaderResourceView* srv = nullptr;

    HRESULT hr = DirectX::CreateWICTextureFromFile(TextureLoader::device, convertedFilePath, &textureResource, &srv);

    if (textureResource) {
        textureResource->Release();
    }

    if (FAILED(hr)) {
        Logger::LOG_WARNING("Failed to load texture: [%s]", filePath.c_str());

        return nullptr;
    } else {
        Logger::LOG_INFO("Loaded texture: [%s]", filePath.c_str());
        textures[filePath] = srv;

        return srv;
    }
}

void TextureLoader::deleteAllTextures()
{
    for (auto srv : textures) {
        srv.second->Release();
    }

    textures.clear();
}
