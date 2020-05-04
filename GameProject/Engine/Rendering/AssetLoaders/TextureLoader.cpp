#include "TextureLoader.hpp"

#include <Engine/Rendering/APIAbstractions/DX11/DeviceDX11.hpp>
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

bool TextureLoader::initHandler()
{
    return true;
}

std::shared_ptr<Texture> TextureLoader::loadTexture(const std::string& filePath)
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
        return nullptr;
    }

    LOG_INFO("Loaded texture: [%s]", filePath.c_str());
    std::shared_ptr<Texture> texture(new Texture(pSRV));
    m_Textures.insert({filePath, texture});

    return texture;
}
