#include "TextureLoader.hpp"

#include <Engine/Rendering/Display.hpp>
#include <Engine/Utils/ECSUtils.hpp>
#include <Engine/Utils/Logger.hpp>

#include <DirectXTK/WICTextureLoader.h>

TextureLoader::TextureLoader(ECSCore* pECS, ID3D11Device* pDevice)
    :m_pDevice(pDevice),
    ComponentHandler({}, pECS, TID(TextureLoader))
{
    ComponentHandlerRegistration handlerReg = {};
    handlerReg.pComponentHandler = this;
    registerHandler(handlerReg);
}

TextureLoader::~TextureLoader()
{
    this->deleteAllTextures();
}

bool TextureLoader::init()
{
    return true;
}

TextureReference TextureLoader::loadTexture(const std::string& filePath)
{
    // See if the texture is already loaded
    auto itr = m_Textures.find(filePath);

    if (itr != m_Textures.end()) {
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

    HRESULT hr = DirectX::CreateWICTextureFromFile(TextureLoader::m_pDevice, convertedFilePath, &textureResource, &pSRV);

    if (textureResource) {
        textureResource->Release();
    }

    if (FAILED(hr)) {
        Log_Warning("Failed to load texture: [%s]", filePath.c_str());
    } else {
        Log_Info("Loaded texture: [%s]", filePath.c_str());
        Texture* pTexture = new Texture(pSRV);
        m_Textures[filePath] = pTexture;

        TextureReference textureReference(pTexture);
        return textureReference;
    }

    return TextureReference(nullptr);
}

void TextureLoader::deleteAllTextures()
{
    for (auto texturePair : m_Textures) {
        delete texturePair.second;
    }

    m_Textures.clear();
}
