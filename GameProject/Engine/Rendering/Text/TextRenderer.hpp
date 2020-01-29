#pragma once

#define NOMINMAX

#include <Engine/ECS/ComponentHandler.hpp>
#include <Engine/Rendering/ShaderHandler.hpp>
#include <DirectXMath.h>
#include <d3d11.h>
#include <map>
#include <memory>
#include <string>
#include <wrl/client.h>

#include <ft2build.h>
#include FT_FREETYPE_H

struct Bitmap {
    unsigned int rows;
    unsigned int width;
    uint8_t* buffer;
};

struct ProcessedGlyph {
    FT_Glyph_Metrics metrics;
    Bitmap bitmap;
};

class TextRenderer : public ComponentHandler
{
public:
    TextRenderer(SystemSubscriber* systemSubscriber, ID3D11Device* device, ID3D11DeviceContext* context);
    ~TextRenderer();

    // Creates a texture with text rendered onto it
    ID3D11ShaderResourceView* renderText(const std::string& text, const std::string& font, unsigned int fontPixelHeight);

private:
    // Loads and maps each character in a string to FreeType glyph data
    bool loadGlyphs(std::map<char, ProcessedGlyph>& glyphs, FT_Face face, const std::string& text, const std::string& font, unsigned int fontPixelHeight);

    // Calculates the size in pixels required by a texture to hold the rendered text
    DirectX::XMUINT2 calculateTextureSize(const std::string& text, const std::map<char, ProcessedGlyph>& glyphs, const FT_Face face);

    // Copies a glyph's bitmap data into the target bitmap's bitmap
    void drawGlyphToTexture(unsigned char* renderTarget, const DirectX::XMUINT2& textureSize, const DirectX::XMUINT2& pen, const Bitmap& glyphBitmap);

    // Convert a bitmap to a texture.Requires that the bitmap is converted into uints.
    ID3D11ShaderResourceView* bitmapToTexture(const Bitmap& bitmap);

    // Convert a bitmap into an array of unsigned integers
    void bitmapToUints(const FT_Bitmap& bitmap, std::vector<uint8_t>& uints);

    ID3D11Device* device;
    ID3D11DeviceContext* context;

    // Maps font names to a mapping of characters to loaded character textures
    std::map<std::string, std::map<char, ID3D11ShaderResourceView*>> loadedCharacters;

    FT_Library ftLib;

    /* Rendering resources */
    Program* uiProgram;

    ID3D11SamplerState *const* aniSampler;

    Microsoft::WRL::ComPtr<ID3D11Buffer> quad;

    // Constant buffer for UI vertex shader
    Microsoft::WRL::ComPtr<ID3D11Buffer> perCharBuffer;
};
