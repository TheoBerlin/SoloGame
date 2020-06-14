#pragma once

#include <Engine/Rendering/APIAbstractions/Texture.hpp>

#define NOMINMAX
#include <d3d11.h>
#include <string>

struct TextureInfoDX11 {
    glm::uvec2 Dimensions;
    RESOURCE_FORMAT Format;
    TEXTURE_LAYOUT LayoutFlags;
    ID3D11ShaderResourceView* pSRV;
    ID3D11DepthStencilView* pDSV;
    ID3D11RenderTargetView* pRTV;
    ID3D11UnorderedAccessView* pUAV;
};

class TextureDX11 : public Texture
{
public:
    static TextureDX11* createFromFile(const std::string& filePath, ID3D11Device* pDevice);
    static TextureDX11* create(const TextureInfo& textureInfo, ID3D11Device* pDevice);

public:
    TextureDX11(const TextureInfoDX11& textureInfo);
    ~TextureDX11();

    void convertTextureLayout(ID3D11DeviceContext* pContext, ID3D11Device* pDevice, TEXTURE_LAYOUT oldLayout, TEXTURE_LAYOUT newLayout);

    ID3D11ShaderResourceView* getSRV() const    { return m_pSRV; }
    ID3D11RenderTargetView* getRTV() const      { return m_pRTV; }
    ID3D11DepthStencilView* getDSV() const      { return m_pDSV; }
    ID3D11Resource* getResource();

    static TEXTURE_LAYOUT convertBindFlags(UINT bindFlags);

private:
    static UINT convertLayoutFlags(TEXTURE_LAYOUT layoutFlags);

private:
    ID3D11ShaderResourceView* m_pSRV;
    ID3D11DepthStencilView* m_pDSV;
    ID3D11RenderTargetView* m_pRTV;
    ID3D11UnorderedAccessView* m_pUAV;

    TEXTURE_LAYOUT m_LayoutFlags;
};
