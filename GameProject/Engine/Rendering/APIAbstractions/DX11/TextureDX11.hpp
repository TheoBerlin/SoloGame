#pragma once

#include <Engine/Rendering/APIAbstractions/Texture.hpp>

#define NOMINMAX
#include <d3d11.h>
#include <string>

class TextureDX11 : public Texture
{
public:
    // Create texture by reading from file
    static TextureDX11* createFromFile(const std::string& filePath, ID3D11Device* pDevice);
    static TextureDX11* create(const TextureInfo& textureInfo, ID3D11Device* pDevice);

public:
    TextureDX11(const glm::uvec2& dimensions, RESOURCE_FORMAT format, ID3D11ShaderResourceView* pSRV, ID3D11DepthStencilView* pDSV, ID3D11RenderTargetView* pRTV, ID3D11UnorderedAccessView* pUAV);
    ~TextureDX11();

    void convertTextureLayout(TEXTURE_LAYOUT oldLayout, TEXTURE_LAYOUT newLayout) override final;

    ID3D11ShaderResourceView* getSRV() const    { return m_pSRV; }
    ID3D11RenderTargetView* getRTV() const      { return m_pRTV; }
    ID3D11DepthStencilView* getDSV() const      { return m_pDSV; }

private:
    static UINT convertBindFlags(TEXTURE_LAYOUT layoutFlags);

private:
    ID3D11ShaderResourceView* m_pSRV;
    ID3D11DepthStencilView* m_pDSV;
    ID3D11RenderTargetView* m_pRTV;
    ID3D11UnorderedAccessView* m_pUAV;
};
