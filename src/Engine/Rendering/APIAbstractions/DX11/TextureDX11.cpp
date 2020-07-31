#include "TextureDX11.hpp"

#include <Engine/Rendering/APIAbstractions/DX11/GeneralResourcesDX11.hpp>
#include <Engine/Utils/Debug.hpp>
#include <Engine/Utils/DirectXUtils.hpp>
#include <Engine/Utils/Logger.hpp>

#include <DirectXTK/WICTextureLoader.h>

#include <wrl/client.h>

TextureDX11* TextureDX11::createFromFile(const std::string& filePath, ID3D11Device* pDevice)
{
    // Convert std::string to const wchar_t*
    std::wstring wStrPath(filePath.begin(), filePath.end());
    const wchar_t* pConvertedFilePath = wStrPath.c_str();

    // Load the texture
    ID3D11ShaderResourceView* pSRV = nullptr;
    ID3D11Resource* pTextureResource = nullptr;

    HRESULT hr = DirectX::CreateWICTextureFromFile(pDevice, pConvertedFilePath, &pTextureResource, &pSRV);
    if (FAILED(hr)) {
        LOG_WARNINGF("Failed to load texture: [%s]", filePath.c_str());
        SAFERELEASE(pTextureResource)
        return nullptr;
    }

    ID3D11Texture2D* tx2D = static_cast<ID3D11Texture2D*>(pTextureResource);
    D3D11_TEXTURE2D_DESC txDesc = {};
    tx2D->GetDesc(&txDesc);
    glm::uvec2 dimensions((uint32_t)txDesc.Width, (uint32_t)txDesc.Height);

    SAFERELEASE(pTextureResource)

    TextureInfoDX11 textureInfoDX = {};
    textureInfoDX.Dimensions    = {(uint32_t)txDesc.Width, (uint32_t)txDesc.Height};
    textureInfoDX.Format        = convertFormatFromDX(txDesc.Format);
    textureInfoDX.pSRV          = pSRV;
    textureInfoDX.pDSV          = nullptr;
    textureInfoDX.pRTV          = nullptr;
    textureInfoDX.pUAV          = nullptr;

    return DBG_NEW TextureDX11(textureInfoDX);
}

TextureDX11* TextureDX11::create(const TextureInfo& textureInfo, ID3D11Device* pDevice)
{
    D3D11_TEXTURE2D_DESC txDesc = {};
    txDesc.Width                = UINT(textureInfo.Dimensions.x);
    txDesc.Height               = UINT(textureInfo.Dimensions.y);
    txDesc.MipLevels            = 1;
    txDesc.ArraySize            = 1;
    txDesc.Format               = convertFormatToDX(textureInfo.Format);
    txDesc.SampleDesc.Count     = 1;
    txDesc.SampleDesc.Quality   = 0;
    txDesc.Usage                = txDesc.BindFlags == D3D11_BIND_SHADER_RESOURCE ? D3D11_USAGE_IMMUTABLE : D3D11_USAGE_DEFAULT;
    txDesc.BindFlags            = TextureDX11::convertUsageMask(textureInfo.Usage);
    txDesc.CPUAccessFlags       = 0;
    txDesc.MiscFlags            = 0;

    D3D11_SUBRESOURCE_DATA initialData = {};
    if (textureInfo.pInitialData) {
        initialData.pSysMem     = textureInfo.pInitialData->pData;
        initialData.SysMemPitch = textureInfo.pInitialData->RowSize;
    }

    Microsoft::WRL::ComPtr<ID3D11Texture2D> texture2D = nullptr;
    HRESULT hr = pDevice->CreateTexture2D(&txDesc, textureInfo.pInitialData ? &initialData : nullptr, texture2D.GetAddressOf());
    if (FAILED(hr)) {
        LOG_WARNINGF("Failed to create texture2D: %s", hresultToString(hr).c_str());
        return nullptr;
    }

    ID3D11Texture2D* pTexture2D = texture2D.Get();
    ID3D11ShaderResourceView* pSRV = nullptr;
    if (txDesc.BindFlags & D3D11_BIND_SHADER_RESOURCE) {
        // Create shader resource view
        hr = pDevice->CreateShaderResourceView(pTexture2D, nullptr, &pSRV);
        if (FAILED(hr)) {
            LOG_WARNINGF("Failed to create shader resource view: %s", hresultToString(hr).c_str());
            return nullptr;
        }
    }

    ID3D11RenderTargetView* pRTV = nullptr;
    if (txDesc.BindFlags & D3D11_BIND_RENDER_TARGET) {
        // Create render target view
        hr = pDevice->CreateRenderTargetView(pTexture2D, nullptr, &pRTV);
        if (FAILED(hr)) {
            LOG_ERRORF("Failed to create render target: %s", hresultToString(hr).c_str());
            return nullptr;
        }
    }

    ID3D11DepthStencilView* pDSV = nullptr;
    if (txDesc.BindFlags & D3D11_BIND_DEPTH_STENCIL) {
        // Create render target view
        hr = pDevice->CreateDepthStencilView(pTexture2D, nullptr, &pDSV);
        if (FAILED(hr)) {
            LOG_ERRORF("Failed to create depth stencil: %s", hresultToString(hr).c_str());
            return nullptr;
        }
    }

    TextureInfoDX11 textureInfoDX = {};
    textureInfoDX.Dimensions    = textureInfo.Dimensions;
    textureInfoDX.Format        = textureInfo.Format;
    textureInfoDX.pSRV          = pSRV;
    textureInfoDX.pDSV          = pDSV;
    textureInfoDX.pRTV          = pRTV;
    textureInfoDX.pUAV          = nullptr;

    return DBG_NEW TextureDX11(textureInfoDX);
}

TextureDX11::TextureDX11(const TextureInfoDX11& textureInfo)
    :Texture(textureInfo.Dimensions, textureInfo.Format),
    m_pSRV(textureInfo.pSRV),
    m_pDSV(textureInfo.pDSV),
    m_pRTV(textureInfo.pRTV),
    m_pUAV(textureInfo.pUAV)
{}

TextureDX11::~TextureDX11()
{
    SAFERELEASE(m_pSRV)
    SAFERELEASE(m_pDSV)
    SAFERELEASE(m_pRTV)
    SAFERELEASE(m_pUAV)
}

ID3D11Resource* TextureDX11::getResource()
{
    ID3D11Resource* pResource = nullptr;

    if (m_pSRV) {
        m_pSRV->GetResource(&pResource);
    } else if (m_pDSV) {
        m_pDSV->GetResource(&pResource);
    } else if (m_pRTV) {
        m_pRTV->GetResource(&pResource);
    } else if (m_pUAV) {
        m_pUAV->GetResource(&pResource);
    }

    return pResource;
}

TEXTURE_LAYOUT TextureDX11::convertBindFlags(UINT bindFlags)
{
    TEXTURE_LAYOUT layoutFlags = {};
    if (HAS_FLAG(bindFlags, D3D11_BIND_SHADER_RESOURCE)) {
        layoutFlags |= TEXTURE_LAYOUT::SHADER_READ_ONLY;
    }

    if (HAS_FLAG(bindFlags, D3D11_BIND_RENDER_TARGET)) {
        layoutFlags |= TEXTURE_LAYOUT::RENDER_TARGET;
    }

    if (HAS_FLAG(bindFlags, D3D11_BIND_DEPTH_STENCIL)) {
        layoutFlags |= TEXTURE_LAYOUT::DEPTH_ATTACHMENT;
    }

    return layoutFlags;
}

UINT TextureDX11::convertUsageMask(TEXTURE_USAGE usage)
{
    return
        HAS_FLAG(usage, TEXTURE_USAGE::SAMPLED)        * D3D11_BIND_SHADER_RESOURCE    |
        HAS_FLAG(usage, TEXTURE_USAGE::STORAGE)        * D3D11_BIND_UNORDERED_ACCESS   |
        HAS_FLAG(usage, TEXTURE_USAGE::RENDER_TARGET)  * D3D11_BIND_RENDER_TARGET      |
        HAS_FLAG(usage, TEXTURE_USAGE::DEPTH_STENCIL)  * D3D11_BIND_DEPTH_STENCIL;
}
