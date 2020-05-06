#include "TextureDX11.hpp"

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

    return new TextureDX11(dimensions, pSRV, nullptr, nullptr, nullptr);
}

TextureDX11* TextureDX11::create(const TextureInfo& textureInfo, ID3D11Device* pDevice)
{
    D3D11_TEXTURE2D_DESC txDesc = {};
    txDesc.Width                = UINT(textureInfo.Dimensions.x);
    txDesc.Height               = UINT(textureInfo.Dimensions.y);
    txDesc.MipLevels            = 1;
    txDesc.ArraySize            = 1;
    txDesc.Format               = TextureDX11::convertFormat(textureInfo.Format);
    txDesc.SampleDesc.Count     = 1;
    txDesc.SampleDesc.Quality   = 0;
    txDesc.Usage                = D3D11_USAGE_DEFAULT;
    txDesc.BindFlags            = TextureDX11::getBindFlags(textureInfo.LayoutFlags);
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

    return new TextureDX11(textureInfo.Dimensions, pSRV, pDSV, pRTV, nullptr);
}

TextureDX11::TextureDX11(const glm::uvec2& dimensions, ID3D11ShaderResourceView* pSRV, ID3D11DepthStencilView* pDSV, ID3D11RenderTargetView* pRTV, ID3D11UnorderedAccessView* pUAV)
    :Texture(dimensions),
    m_pSRV(pSRV),
    m_pDSV(pDSV),
    m_pRTV(pRTV),
    m_pUAV(pUAV)
{}

TextureDX11::~TextureDX11()
{
    SAFERELEASE(m_pSRV)
    SAFERELEASE(m_pDSV)
    SAFERELEASE(m_pRTV)
    SAFERELEASE(m_pUAV)
}

void TextureDX11::convertTextureLayout(TEXTURE_LAYOUT oldLayout, TEXTURE_LAYOUT newLayout)
{
    // DX11 does texture conversions implicitly :)
}

DXGI_FORMAT TextureDX11::convertFormat(TEXTURE_FORMAT textureFormat)
{
    switch (textureFormat) {
        case TEXTURE_FORMAT::R8G8B8A8_UNORM:
            return DXGI_FORMAT_R8G8B8A8_UNORM;
        case TEXTURE_FORMAT::D32_FLOAT:
            return DXGI_FORMAT_D32_FLOAT;
        default:
            LOG_ERROR("Unknown texture format");
            return DXGI_FORMAT_UNKNOWN;
    }
}

UINT TextureDX11::getBindFlags(TEXTURE_LAYOUT layoutFlags)
{
    return
        HAS_FLAG(layoutFlags, TEXTURE_LAYOUT::SHADER_READ_ONLY) * D3D11_BIND_SHADER_RESOURCE |
        HAS_FLAG(layoutFlags, TEXTURE_LAYOUT::RENDER_TARGET)    * D3D11_BIND_RENDER_TARGET |
        HAS_FLAG(layoutFlags, TEXTURE_LAYOUT::DEPTH_ATTACHMENT) * D3D11_BIND_DEPTH_STENCIL;
}