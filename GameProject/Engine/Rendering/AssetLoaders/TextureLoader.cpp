#include "TextureLoader.hpp"

#include <Engine/Rendering/APIAbstractions/DX11/DeviceDX11.hpp>
#include <Engine/Rendering/AssetContainers/TextureReference.hpp>
#include <Engine/Utils/ECSUtils.hpp>
#include <Engine/Utils/Logger.hpp>

#include <d3d11.h>
#include <DirectXTK/WICTextureLoader.h>

TextureLoader::TextureLoader(ECSCore* pECS, IDevice* pDevice)
    :ComponentHandler(pECS, TID(TextureLoader)),
    m_pDevice(pDevice)
{
    ComponentHandlerRegistration handlerReg = {};
    handlerReg.pComponentHandler = this;
    registerHandler(handlerReg);
}

TextureLoader::~TextureLoader()
{
    this->deleteAllTextures();
}

bool TextureLoader::initHandler()
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

    DeviceDX11* pDevice = reinterpret_cast<DeviceDX11*>(m_pDevice);

    HRESULT hr = DirectX::CreateWICTextureFromFile(pDevice->getDevice(), convertedFilePath, &textureResource, &pSRV);

    if (textureResource) {
        textureResource->Release();
    }

    if (FAILED(hr)) {
        LOG_WARNING("Failed to load texture: [%s]", filePath.c_str());
        return TextureReference(nullptr);
    }

    LOG_INFO("Loaded texture: [%s]", filePath.c_str());
    Texture* pTexture = new Texture(pSRV);
    m_Textures[filePath] = pTexture;

    TextureReference textureReference(pTexture);
    return textureReference;
}

void TextureLoader::deleteAllTextures()
{
    for (auto texturePair : m_Textures) {
        delete texturePair.second;
    }

    m_Textures.clear();
}
