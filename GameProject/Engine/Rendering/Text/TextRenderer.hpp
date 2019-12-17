#pragma once

#define NOMINMAX

#include <Engine/ECS/ComponentHandler.hpp>
#include <Engine/Rendering/ShaderHandler.hpp>
#include <DirectXMath.h>
#include <d3d11.h>
#include <map>
#include <wrl/client.h>

#include <ft2build.h>
#include FT_FREETYPE_H

class TextRenderer : public ComponentHandler
{
public:
    TextRenderer(SystemSubscriber* systemSubscriber, ID3D11Device* device, ID3D11DeviceContext* context);
    ~TextRenderer();

    // Creates a texture with text rendered onto it
    ID3D11ShaderResourceView* renderText(const std::string& text, const std::string& font, unsigned int fontPixelHeight);

private:
    // Loads and maps each character in a string to FreeType glyph data
    bool loadGlyphs(FT_Face face, const std::string& text, const std::string& font, unsigned int fontPixelHeight, std::map<char, FT_GlyphSlotRec_>* glyphs);

    // Calculates the size in pixels required by a texture to hold the rendered text
    DirectX::XMUINT2 calculateTextureSize(const std::string& text, const std::map<char, FT_GlyphSlotRec_>& glyphs);

    // Render a glyph onto a texture
    Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> glyphToTexture(char character, const FT_GlyphSlot glyph);

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
