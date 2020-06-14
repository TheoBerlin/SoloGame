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
        LOG_WARNING("Failed to load texture: [%s]", filePath.c_str());
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
    textureInfoDX.LayoutFlags   = convertBindFlags(txDesc.BindFlags);
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
    txDesc.Usage                = D3D11_USAGE_DEFAULT;
    txDesc.BindFlags            = TextureDX11::convertLayoutFlags(textureInfo.LayoutFlags);
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
        LOG_WARNING("Failed to create texture for UI panel: %s", hresultToString(hr).c_str());
        return nullptr;
    }

    ID3D11Texture2D* pTexture2D = texture2D.Get();
    ID3D11ShaderResourceView* pSRV = nullptr;
    if (HAS_FLAG(textureInfo.LayoutFlags, TEXTURE_LAYOUT::SHADER_READ_ONLY)) {
        // Create shader resource view
        hr = pDevice->CreateShaderResourceView(pTexture2D, nullptr, &pSRV);
        if (FAILED(hr)) {
            LOG_WARNING("Failed to create shader resource view for UI panel: %s", hresultToString(hr).c_str());
            return nullptr;
        }
    }

    ID3D11RenderTargetView* pRTV = nullptr;
    if (HAS_FLAG(textureInfo.LayoutFlags, TEXTURE_LAYOUT::RENDER_TARGET)) {
        // Create render target view
        hr = pDevice->CreateRenderTargetView(pTexture2D, nullptr, &pRTV);
        if (FAILED(hr)) {
            LOG_ERROR("Failed to create render target: %s", hresultToString(hr).c_str());
            return nullptr;
        }
    }

    ID3D11DepthStencilView* pDSV = nullptr;
    if (HAS_FLAG(textureInfo.LayoutFlags, TEXTURE_LAYOUT::DEPTH_ATTACHMENT)) {
        // Create render target view
        hr = pDevice->CreateDepthStencilView(pTexture2D, nullptr, &pDSV);
        if (FAILED(hr)) {
            LOG_ERROR("Failed to create depth stencil: %s", hresultToString(hr).c_str());
            return nullptr;
        }
    }

    TextureInfoDX11 textureInfoDX = {};
    textureInfoDX.Dimensions    = textureInfo.Dimensions;
    textureInfoDX.Format        = textureInfo.Format;
    textureInfoDX.LayoutFlags   = textureInfo.LayoutFlags;
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
    m_pUAV(textureInfo.pUAV),
    m_LayoutFlags(textureInfo.LayoutFlags)
{}

TextureDX11::~TextureDX11()
{
    SAFERELEASE(m_pSRV)
    SAFERELEASE(m_pDSV)
    SAFERELEASE(m_pRTV)
    SAFERELEASE(m_pUAV)
}

void TextureDX11::convertTextureLayout(ID3D11DeviceContext* pContext, ID3D11Device* pDevice, TEXTURE_LAYOUT oldLayout, TEXTURE_LAYOUT newLayout)
{
    if (HAS_FLAG(m_LayoutFlags, newLayout)) {
        return;
    }

    // Create a new texture with the old and the new bind flags combined
    m_LayoutFlags = oldLayout | newLayout;

    // Get resource pointer
    ID3D11Resource* pResource = getResource();

    // Get old texture description and only change the bind flags
    D3D11_TEXTURE2D_DESC txDesc = {};
    reinterpret_cast<ID3D11Texture2D*>(pResource)->GetDesc(&txDesc);
    txDesc.BindFlags = convertLayoutFlags(m_LayoutFlags);

    ID3D11Texture2D* pNewTexture = nullptr;
    HRESULT hr = pDevice->CreateTexture2D(&txDesc, nullptr, &pNewTexture);
    if (FAILED(hr)) {
        LOG_WARNING("Failed to create new texture whilst converting texture layout: %s", hresultToString(hr).c_str());
        return;
    }

    pContext->CopyResource(pResource, pNewTexture);

    // Create new texture view, the type depends on the new layout flag
    hr = S_OK;

    switch (newLayout) {
        case TEXTURE_LAYOUT::SHADER_READ_ONLY:
            hr = pDevice->CreateShaderResourceView(pNewTexture, nullptr, &m_pSRV);
            break;
        case TEXTURE_LAYOUT::DEPTH_ATTACHMENT:
            hr = pDevice->CreateDepthStencilView(pNewTexture, nullptr, &m_pDSV);
            break;
        case TEXTURE_LAYOUT::RENDER_TARGET:
            hr = pDevice->CreateRenderTargetView(pNewTexture, nullptr, &m_pRTV);
            break;
        // TODO: UAV support
    }

    if (FAILED(hr)) {
        LOG_WARNING("Failed to create new texture view after conversion, old and new layouts: %d -> %d", (int)oldLayout, (int)newLayout);
    }
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

UINT TextureDX11::convertLayoutFlags(TEXTURE_LAYOUT layoutFlags)
{
    return
        HAS_FLAG(layoutFlags, TEXTURE_LAYOUT::SHADER_READ_ONLY) * D3D11_BIND_SHADER_RESOURCE |
        HAS_FLAG(layoutFlags, TEXTURE_LAYOUT::RENDER_TARGET)    * D3D11_BIND_RENDER_TARGET |
        HAS_FLAG(layoutFlags, TEXTURE_LAYOUT::DEPTH_ATTACHMENT) * D3D11_BIND_DEPTH_STENCIL;
}
