#pragma once

#define NOMINMAX

#include <Engine/ECS/ComponentHandler.hpp>
#include <Engine/Rendering/AssetContainers/Texture.hpp>
#include <Engine/Rendering/ShaderHandler.hpp>

#include <DirectXMath.h>
#include <d3d11.h>
#include <map>
#include <memory>
#include <string>
#include <wrl/client.h>

#include <ft2build.h>
#include FT_FREETYPE_H

struct Bytemap {
    unsigned int rows;
    unsigned int width;
    std::vector<uint8_t> buffer;
};

struct ProcessedGlyph {
    FT_Glyph_Metrics metrics;
    Bytemap bytemap;
};

class IDevice;

class TextRenderer : public ComponentHandler
{
public:
    TextRenderer(ECSCore* pECS, IDevice* pDevice);
    ~TextRenderer();

    virtual bool initHandler() override;

    // Creates a texture with text rendered onto it
    std::shared_ptr<Texture> renderText(const std::string& text, const std::string& font, unsigned int fontPixelHeight);

private:
    // Loads and maps each character in a string to FreeType glyph data
    bool loadGlyphs(std::map<char, ProcessedGlyph>& glyphs, FT_Face face, const std::string& text, const std::string& font, unsigned int fontPixelHeight);

    // Calculates the size in pixels required by a texture to hold the rendered text
    DirectX::XMUINT2 calculateTextureSize(const std::string& text, const std::map<char, ProcessedGlyph>& glyphs, const FT_Face face);

    // Copies a glyph's bitmap data into the target bitmap's bitmap
    void drawGlyphToTexture(unsigned char* renderTarget, const DirectX::XMUINT2& textureSize, const DirectX::XMUINT2& pen, const Bytemap& glyphBytemap);

    // Convert a bytemap to an appropriate format, and create a texture from the results
    ID3D11ShaderResourceView* bytemapToTexture(const Bytemap& bitmap);

    // Convert a bitmap into a bytemap
    void bitmapToBytemap(const FT_Bitmap& bitmap, Bytemap& bytemap);

private:
    IDevice* m_pDevice;

    // Rendering text results in a texture and a texture reference being created. This is the storage for the textures.
    std::vector<std::weak_ptr<Texture>> m_Textures;

    FT_Library ftLib;

    /* Rendering resources */
    Program* uiProgram;

    ID3D11SamplerState *const* aniSampler;

    Microsoft::WRL::ComPtr<ID3D11Buffer> quad;

    // Constant buffer for UI vertex shader
    Microsoft::WRL::ComPtr<ID3D11Buffer> perCharBuffer;
};
