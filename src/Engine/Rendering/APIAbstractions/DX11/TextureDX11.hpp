#pragma once

#include <Engine/Rendering/APIAbstractions/Texture.hpp>

#include <d3d11.h>
#include <string>

struct TextureInfoDX11 {
    glm::uvec2 Dimensions;
    RESOURCE_FORMAT Format;
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

    ID3D11ShaderResourceView* getSRV() const    { return m_pSRV; }
    ID3D11RenderTargetView* getRTV() const      { return m_pRTV; }
    ID3D11DepthStencilView* getDSV() const      { return m_pDSV; }
    ID3D11Resource* getResource();

    static TEXTURE_LAYOUT convertBindFlags(UINT bindFlags);

private:
    static UINT convertUsageMask(TEXTURE_USAGE usage);

private:
    ID3D11ShaderResourceView* m_pSRV;
    ID3D11DepthStencilView* m_pDSV;
    ID3D11RenderTargetView* m_pRTV;
    ID3D11UnorderedAccessView* m_pUAV;
};
