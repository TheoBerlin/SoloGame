#include "TextRenderer.hpp"

#include <DirectXTK/WICTextureLoader.h>
#include <Engine/ECS/SystemSubscriber.hpp>
#include <Engine/Rendering/ShaderResourceHandler.hpp>
#include <Engine/UI/Panel.hpp>
#include <Engine/Utils/DirectXUtils.hpp>
#include <Engine/Utils/Logger.hpp>
#include <algorithm>

TextRenderer::TextRenderer(SystemSubscriber* systemSubscriber, ID3D11Device* device, ID3D11DeviceContext* context)
    :ComponentHandler({}, systemSubscriber, std::type_index(typeid(TextRenderer))),
    device(device),
    context(context)
{
    // https://www.freetype.org/freetype2/docs/reference/ft2-base_interface.html
    // [Since 2.5.6] In multi-threaded applications it is easiest to use one FT_Library object per thread.
    // In case this is too cumbersome, a single FT_Library object across threads is possible also, as long as a mutex lock is used around FT_New_Face and FT_Done_Face.

    // Initialize FreeType library
	FT_Error err = FT_Init_FreeType(&ftLib);
	if (err) {
		Logger::LOG_ERROR("Failed to initialize FreeType library: %s", FT_Error_String(err));
        FT_Done_FreeType(ftLib);
        systemSubscriber->deregisterComponents(this);
        return;
	}

    Logger::LOG_INFO("Initialized FreeType library");
}

TextRenderer::~TextRenderer()
{
    // Release text textures
    for (auto font : loadedCharacters) {
        for (auto characterTexture : font.second) {
            characterTexture.second->Release();
        }
    }

    FT_Error err = FT_Done_FreeType(ftLib);
	if (err) {
		Logger::LOG_ERROR("Failed to release FreeType library: %s", FT_Error_String(err));
	}
}

ID3D11ShaderResourceView* TextRenderer::renderText(const std::string& text, const std::string& font, unsigned int fontPixelHeight)
{
    FT_Face face;
    FT_Error err;

    err = FT_New_Face(ftLib, font.c_str(), 0, &face);
    if (err) {
        Logger::LOG_WARNING("Failed to load FreeType face from file [%s]: %s", font.c_str(), FT_Error_String(err));
        return nullptr;
    }

    // Set the desired size of the font
    err = FT_Set_Pixel_Sizes(face, 0, fontPixelHeight);
    if (err) {
        Logger::LOG_WARNING("[%s] FreeType failed to set pixel size %d: %s", font.c_str(), fontPixelHeight, FT_Error_String(err));
        return nullptr;
    }

    // Create texture to render all characters to
    std::map<char, ProcessedGlyph> glyphs;
    if (!loadGlyphs(glyphs, face, text, font, fontPixelHeight)) {
        return nullptr;
    }

    DirectX::XMUINT2 textureSize = calculateTextureSize(text, glyphs, face);

    // The bytemap to paste glyph bytemaps onto
    Bytemap textBytemap;
    textBytemap.buffer.resize(textureSize.x * textureSize.y);
    ZeroMemory(textBytemap.buffer.data(), textBytemap.buffer.size());
    textBytemap.rows = textureSize.y;
    textBytemap.width = textureSize.x;

    // Check if the font has already been loaded, otherwise, load it
    auto fontItr = loadedCharacters.find(font);

    // Describes where to draw the next character onto the texture
    DirectX::XMUINT2 pen = {0, textureSize.y * 64 - face->size->metrics.ascender};

    // Render each character onto the final texture
    int charIdx = 0;

    for (char character : text) {
        // TODO: Handle newlines
        /*if (character == '\n') {
            pen.x = 0;
            pen.y += ...;
            continue;
        }*/

        if (character == ' ') {
            Logger::LOG_INFO("Max advance (rendering): %d", face->size->metrics.max_advance);
            pen.x += face->size->metrics.max_advance;
            continue;
        }

        auto glyphsItr = glyphs.find(character);
        if (glyphsItr == glyphs.end()) {
            Logger::LOG_WARNING("Expected glyph to be loaded: '%c'", character);
            return nullptr;
        }

        FT_Glyph_Metrics& metrics = glyphsItr->second.metrics;

        // The first letter in a row should not have horizontal bearing
        // TODO: To support multiple rows, this should be done at the beginning of each row
        pen.x -= (charIdx == 0) * metrics.horiBearingX;

        pen.x += metrics.horiBearingX;
        pen.y += metrics.horiBearingY - metrics.height;

        drawGlyphToTexture(textBytemap.buffer.data(), textureSize, {pen.x / 64, pen.y / 64}, glyphsItr->second.bytemap);

        pen.x -= metrics.horiBearingX;
        pen.y -= metrics.horiBearingY - metrics.height;

        // Advance pen
        pen.x += metrics.horiAdvance;

        charIdx += 1;
    }

    return bytemapToTexture(textBytemap);
}

bool TextRenderer::loadGlyphs(std::map<char, ProcessedGlyph>& glyphs, FT_Face face, const std::string& text, const std::string& font, unsigned int fontPixelHeight)
{
    FT_Error err;

    for (char character : text) {
        if (character == ' ' || character == '\n') {
            continue;
        }

        if (glyphs.find(character) != glyphs.end()) {
            // Character is already loaded
            continue;
        }

        err = FT_Load_Char(face, character, FT_LOAD_RENDER);
        if (err) {
            Logger::LOG_WARNING("Failed to load character: '%c', font: %s", character, font.c_str());
            return false;
        }

        ProcessedGlyph processedGlyph;
        processedGlyph.metrics = face->glyph->metrics;

        bitmapToBytemap(face->glyph->bitmap, processedGlyph.bytemap);
        glyphs.insert({character, processedGlyph});
    }

    return true;
}

DirectX::XMUINT2 TextRenderer::calculateTextureSize(const std::string& text, const std::map<char, ProcessedGlyph>& glyphs, const FT_Face face)
{
    DirectX::XMUINT2 textureSize = {0, (uint32_t)face->size->metrics.height}; 

    size_t glyphIdx = 0;

    for (char character : text) {
        if (character == ' ') {
            Logger::LOG_INFO("Max advance (calculate size): %d", face->size->metrics.max_advance);
            textureSize.x += (uint32_t)face->size->metrics.max_advance;
            continue;
        }

        /* To support multiple rows, a 'currentRowWidth' variable is needed, and the maximum of the row widths should be set as textureSize.x
        if (character == '\n') {
            textureSize.y += face->size->metrics.ascender;
            continue;
        }
        */

        auto glyphsItr = glyphs.find(character);
        if (glyphsItr == glyphs.end()) {
            Logger::LOG_WARNING("Expected glyph to be loaded: '%c'", character);
            return textureSize;
        }

        const FT_Glyph_Metrics& glyphMetrics = glyphsItr->second.metrics;

        if (glyphIdx == text.size() - 1) {
            // Exclude right side bearing
            textureSize.x += glyphMetrics.horiBearingX + glyphMetrics.width;
        } else {
            // 'Normal case': All letters that are neither the last nor the first in the string
            textureSize.x += glyphMetrics.horiAdvance;
        }

        if (glyphIdx == 0) {
            textureSize.x -= glyphMetrics.horiBearingX;
        }

        glyphIdx += 1;
    }

    textureSize.x = (uint32_t)(ceil((float)textureSize.x / 64.0f));
    textureSize.y = (uint32_t)(ceil((float)textureSize.y / 64.0f));
    Logger::LOG_INFO("Texture size: (%d, %d)", textureSize.x, textureSize.y);
    return textureSize;
}

void TextRenderer::drawGlyphToTexture(uint8_t* renderTarget, const DirectX::XMUINT2& textureSize, const DirectX::XMUINT2& pen, const Bytemap& glyphBytemap)
{
    // Pen describes the bottom left corner of the glyph's bounding box, but data is copied top to bottom
    unsigned int renderTargetIdx = (textureSize.y - glyphBytemap.rows - pen.y) * textureSize.x + pen.x;
    const uint8_t* glyphBuffer = glyphBytemap.buffer.data();

    for (unsigned int row = 0; row < glyphBytemap.rows; row++) {
        std::memcpy(&renderTarget[renderTargetIdx], &glyphBuffer[row * glyphBytemap.width], glyphBytemap.width);

        renderTargetIdx += textureSize.x;
    }
}

ID3D11ShaderResourceView* TextRenderer::bytemapToTexture(const Bytemap& bytemap)
{
    // Duplicate each byte in the bytemap four times to create an R8G8B8A8 texture
    Bytemap convertedBytemap;
    convertedBytemap.rows = bytemap.rows;
    convertedBytemap.width = bytemap.width;
    convertedBytemap.buffer.reserve(bytemap.buffer.size() * 4);

    for (const uint8_t& byte : bytemap.buffer) {
        // Red
        convertedBytemap.buffer.push_back(byte);
        // Green
        convertedBytemap.buffer.push_back(byte);
        // Blue
        convertedBytemap.buffer.push_back(byte);
        // Alpha
        convertedBytemap.buffer.push_back(byte);
    }

    // Convert the glyph's bytemap into unsigned integers
    D3D11_TEXTURE2D_DESC txDesc;
    ZeroMemory(&txDesc, sizeof(D3D11_TEXTURE2D_DESC));
    txDesc.Width = convertedBytemap.width;
    txDesc.Height = convertedBytemap.rows;
    txDesc.MipLevels = 1;
    txDesc.ArraySize = 1;
    txDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
    txDesc.SampleDesc.Count = 1;
    txDesc.SampleDesc.Quality = 0;
    txDesc.Usage = D3D11_USAGE_DEFAULT;
    txDesc.BindFlags = D3D11_BIND_SHADER_RESOURCE;
    txDesc.CPUAccessFlags = 0;
    txDesc.MiscFlags = 0;

    D3D11_SUBRESOURCE_DATA glyphTxSubdata;
    ZeroMemory(&glyphTxSubdata, sizeof(D3D11_SUBRESOURCE_DATA));
    glyphTxSubdata.pSysMem = convertedBytemap.buffer.data();
    glyphTxSubdata.SysMemPitch = convertedBytemap.width * 4 * sizeof(uint8_t);
    glyphTxSubdata.SysMemSlicePitch = 0;

    Microsoft::WRL::ComPtr<ID3D11Texture2D> glyphTexture;
    HRESULT hr = device->CreateTexture2D(&txDesc, &glyphTxSubdata, glyphTexture.GetAddressOf());
    if (FAILED(hr)) {
        Logger::LOG_WARNING("Failed to create texture from bytemap: %s", hresultToString(hr).c_str());
        return nullptr;
    }

    ID3D11ShaderResourceView* glyphSRV;
    hr = device->CreateShaderResourceView(glyphTexture.Get(), nullptr, &glyphSRV);
    if (FAILED(hr)) {
        Logger::LOG_WARNING("Failed to create shader resource view from bytemap: %s", hresultToString(hr).c_str());
        return nullptr;
    }

    return glyphSRV;
}

void TextRenderer::bitmapToBytemap(const FT_Bitmap& bitmap, Bytemap& bytemap)
{
    // The bitmap's pitch might be negative in case the first bytes describe the bottom row of the image. This might need to be handled.
    if (bitmap.pitch < 0) {
        Logger::LOG_WARNING("Pitch is negative, this is not handled correctly!");
    }

    bytemap.rows = bitmap.rows;
    bytemap.width = bitmap.width;

    // Pitch is the amount of bytes per row
    size_t pixelCount = (size_t)(bitmap.rows * bitmap.width);
    bytemap.buffer.resize(pixelCount);

    const unsigned char* bitmapBuffer = bitmap.buffer;

    if (bitmap.pixel_mode == FT_PIXEL_MODE_MONO) {
        // Binary format, 1 bit is one pixel
        for (size_t bitIdx = 0; bitIdx < pixelCount; bitIdx += 1) {
            // Convert 8 bits into 8 uints
            unsigned char bit = bitmapBuffer[bitIdx];

            for (size_t i = 0; i < 8; i += 1) {
                bytemap.buffer[bitIdx * 8 + i] = (bit >> i) & (unsigned)1;
            }
        }
    } else if (bitmap.pixel_mode == FT_PIXEL_MODE_GRAY) {
        // Grayscale bytemap, copy into target buffer
        std::memcpy((void*)bytemap.buffer.data(), (const void*)bitmapBuffer, bytemap.buffer.size());
    }
}
