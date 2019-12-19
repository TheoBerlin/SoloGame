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
    // Retrieve UI shader program
    const std::type_index tid_shaderHandler = std::type_index(typeid(ShaderHandler));
    ShaderHandler* shaderHandler = static_cast<ShaderHandler*>(systemSubscriber->getComponentHandler(tid_shaderHandler));

    this->uiProgram = shaderHandler->getProgram(PROGRAM::UI);

    // Retrieve quad vertex buffer
    const std::type_index tid_shaderResourceHandler = std::type_index(typeid(ShaderResourceHandler));
    ShaderResourceHandler* shaderResourceHandler = static_cast<ShaderResourceHandler*>(systemSubscriber->getComponentHandler(tid_shaderResourceHandler));
    this->quad = shaderResourceHandler->getQuarterScreenQuad();
    this->aniSampler = shaderResourceHandler->getAniSampler();

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

    // Create per-character constant buffer
    D3D11_BUFFER_DESC bufferDesc;
    ZeroMemory(&bufferDesc, sizeof(D3D11_BUFFER_DESC));
    bufferDesc.ByteWidth = sizeof(UIPanel) - sizeof(ID3D11ShaderResourceView*);
    bufferDesc.Usage = D3D11_USAGE_DYNAMIC;
    bufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
    bufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
    bufferDesc.MiscFlags = 0;
    bufferDesc.StructureByteStride = 0;

    HRESULT hr = device->CreateBuffer(&bufferDesc, nullptr, perCharBuffer.GetAddressOf());
    if (FAILED(hr))
        Logger::LOG_ERROR("Failed to create per-char cbuffer: %s", hresultToString(hr).c_str());
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

    D3D11_TEXTURE2D_DESC txDesc;
    ZeroMemory(&txDesc, sizeof(D3D11_TEXTURE2D_DESC));
    txDesc.Width = textureSize.x;
    txDesc.Height = textureSize.y;
    txDesc.MipLevels = 1;
    txDesc.ArraySize = 1;
    txDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
    txDesc.SampleDesc.Count = 1;
    txDesc.SampleDesc.Quality = 0;
    txDesc.Usage = D3D11_USAGE_DEFAULT;
    txDesc.BindFlags = D3D11_BIND_SHADER_RESOURCE | D3D11_BIND_RENDER_TARGET;
    txDesc.CPUAccessFlags = 0;
    txDesc.MiscFlags = 0;

    Microsoft::WRL::ComPtr<ID3D11Texture2D> finalTexture;
    HRESULT hr = device->CreateTexture2D(&txDesc, nullptr, finalTexture.GetAddressOf());
    if (FAILED(hr)) {
        Logger::LOG_WARNING("Failed to create texture for text: %s", hresultToString(hr).c_str());
        return nullptr;
    }

    D3D11_RENDER_TARGET_VIEW_DESC rtvDesc;
    ZeroMemory(&rtvDesc, sizeof(D3D11_RENDER_TARGET_VIEW_DESC));
    rtvDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
    rtvDesc.ViewDimension = D3D11_RTV_DIMENSION_TEXTURE2D;
    rtvDesc.Texture2D.MipSlice = 0;

    Microsoft::WRL::ComPtr<ID3D11RenderTargetView> finalRTV;
    hr = device->CreateRenderTargetView(finalTexture.Get(), &rtvDesc, finalRTV.GetAddressOf());
    if (FAILED(hr)) {
        Logger::LOG_WARNING("Failed to create render target view for text: %s, error: %s", text.c_str(), hresultToString(hr).c_str());
        return nullptr;
    }

    // Check if the font has already been loaded, otherwise, load it
    auto fontItr = loadedCharacters.find(font);

    // Initial rendering setup
    context->VSSetShader(uiProgram->vertexShader, nullptr, 0);
    context->HSSetShader(uiProgram->hullShader, nullptr, 0);
    context->DSSetShader(uiProgram->domainShader, nullptr, 0);
    context->GSSetShader(uiProgram->geometryShader, nullptr, 0);
    context->PSSetShader(uiProgram->pixelShader, nullptr, 0);

    context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP);
    context->IASetInputLayout(uiProgram->inputLayout);
    UINT offsets = 0;
    context->IASetVertexBuffers(0, 1, quad.GetAddressOf(), &uiProgram->vertexSize, &offsets);

    context->PSSetSamplers(0, 1, aniSampler);
    context->OMSetRenderTargets(1, finalRTV.GetAddressOf(), nullptr);

    // Used for editing per-character constant buffer
    D3D11_MAPPED_SUBRESOURCE mappedResources;
    ZeroMemory(&mappedResources, sizeof(D3D11_MAPPED_SUBRESOURCE));

    // Describes where to draw the next character onto the texture
    DirectX::XMFLOAT2 pen = {0.0f, (float)(textureSize.y - face->size->metrics.ascender)};

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

        Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> glyphSRV(glyphsItr->second.texture);
        FT_Glyph_Metrics& metrics = glyphsItr->second.metrics;

        pen.x -= (charIdx == 0) * metrics.horiBearingX;

        // Draw character to string texture
        UIPanel charPanel;
        // Add horizontal bearing for every character but the first
        // TODO: Do this for the first character of each line
        charPanel.position.x = (pen.x + metrics.horiBearingX) / ((float)textureSize.x/* * 64.0f*/);
        charPanel.position.y = (pen.y + metrics.horiBearingY - metrics.height) / ((float)face->size->metrics.ascender/* * 64.0f*/);
        charPanel.size.x = metrics.width / ((float)textureSize.x/* * 64.0f*/);
        charPanel.size.y = metrics.height / ((float)textureSize.y/* * 64.0f*/);
        charPanel.color = {1.0f, 1.0f, 1.0f, 1.0f};

        // Set per-char buffer
        context->Map(perCharBuffer.Get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResources);
        memcpy(mappedResources.pData, &charPanel, sizeof(UIPanel) - sizeof(ID3D11ShaderResourceView*));
        context->Unmap(perCharBuffer.Get(), 0);

        context->VSSetConstantBuffers(0, 1, perCharBuffer.GetAddressOf());
        context->PSSetConstantBuffers(0, 1, perCharBuffer.GetAddressOf());

        context->PSSetShaderResources(0, 1, glyphSRV.GetAddressOf());

        context->Draw(4, 0);

        // Advance pen
        pen.x += metrics.horiAdvance;

        charIdx += 1;
    }

    // TODO: Handle return, change return type to ComPtr?
    // Create SRV of finished texture and return it
    ID3D11ShaderResourceView* finishedSRV = nullptr;
    hr = device->CreateShaderResourceView(finalTexture.Get(), nullptr, &finishedSRV);
    if (FAILED(hr)) {
        Logger::LOG_WARNING("Failed to create shader resource view finished text texture: %s", hresultToString(hr).c_str());
        return nullptr;
    }

    // Prevent texture from being deleted after exiting function NOT NEEDED?
    //finalTexture.Detach();

    return finishedSRV;
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

        ID3D11ShaderResourceView* glyphTexture = glyphToTexture(character, face->glyph);

        glyphs.insert({character, {face->glyph->metrics, glyphTexture}});
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

        /* To support multiple rows, a 'currentRowWidth' variable is needed
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
        //textureSize.x += glyphMetrics.width;
        // TODO: Don't use advance for the last character in the text (or line), instead, use left bearing + width (this exludes the right side bearing)
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

    //textureSize.x = (unsigned int)(ceil((float)textureSize.x / 64.0f));
    //textureSize.y = (unsigned int)(ceil((float)textureSize.y / 64.0f));
    return textureSize;
}

ID3D11ShaderResourceView* TextRenderer::glyphToTexture(char character, const FT_GlyphSlot glyph)
{
    if (glyph->format != FT_GLYPH_FORMAT_BITMAP) {
        Logger::LOG_WARNING("Cannot convert glyph to texture: glyph is not expressed using a bitmap");
    }

    // Convert the glyph's bitmap into unsigned integers
    std::vector<uint8_t> uints;
    bitmapToUints(glyph->bitmap, uints);

    D3D11_TEXTURE2D_DESC txDesc;
    ZeroMemory(&txDesc, sizeof(D3D11_TEXTURE2D_DESC));
    txDesc.Width = glyph->bitmap.width;
    txDesc.Height = glyph->bitmap.rows;
    txDesc.MipLevels = 1;
    txDesc.ArraySize = 1;
    txDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
    txDesc.SampleDesc.Count = 1;
    txDesc.SampleDesc.Quality = 0;
    txDesc.Usage = D3D11_USAGE_DEFAULT;
    txDesc.BindFlags = D3D11_BIND_SHADER_RESOURCE;
    txDesc.CPUAccessFlags = 0;
    txDesc.MiscFlags = 0;

    // Use character bitmap to generate D3D11 texture
    D3D11_SUBRESOURCE_DATA glyphTxSubdata;
    ZeroMemory(&glyphTxSubdata, sizeof(D3D11_SUBRESOURCE_DATA));
    glyphTxSubdata.pSysMem = &uints.front();
    // The bitmap's pitch might be negative in case the first bytes describe the bottom row of the image. This might need to be handled.
    if (glyph->bitmap.pitch < 0) {
        Logger::LOG_WARNING("Pitch is negative for character: %c", character);
    }
    glyphTxSubdata.SysMemPitch = glyph->bitmap.width * sizeof(uint8_t) * 4;
    glyphTxSubdata.SysMemSlicePitch = 0;

    Microsoft::WRL::ComPtr<ID3D11Texture2D> glyphTexture;
    HRESULT hr = device->CreateTexture2D(&txDesc, &glyphTxSubdata, glyphTexture.GetAddressOf());
    if (FAILED(hr)) {
        Logger::LOG_WARNING("Failed to create texture for character: %c, error: %s", character, hresultToString(hr).c_str());
        return nullptr;
    }

    ID3D11ShaderResourceView* glyphSRV;
    hr = device->CreateShaderResourceView(glyphTexture.Get(), nullptr, &glyphSRV);
    if (FAILED(hr)) {
        Logger::LOG_WARNING("Failed to create shader resource view for character: '%c', error: %s", character, hresultToString(hr).c_str());
        return nullptr;
    }

    return glyphSRV;
}

void TextRenderer::bitmapToUints(const FT_Bitmap& bitmap, std::vector<uint8_t>& uints)
{
    // Pitch is the amount of bytes per row
    size_t bitmapByteSize = (size_t)(bitmap.pitch * bitmap.rows); // not needed?
    size_t pixelCount = (size_t)(bitmap.rows * bitmap.width);
    uints.reserve(pixelCount);

    const unsigned char* bitmapBuffer = bitmap.buffer;

    if (bitmap.pixel_mode == FT_PIXEL_MODE_MONO) {
        for (size_t bitIdx = 0; bitIdx < pixelCount; bitIdx += 1) {
            // Convert 8 bits into 8 uints
            unsigned char bit = bitmapBuffer[bitIdx];

            for (size_t i = 0; i < 8; i += 1) {
                // Red
                uints.push_back((bit >> i) & (unsigned)1);
                // Green
                uints.push_back(uints.back());
                // Blue
                uints.push_back(uints.back());
                // Alpha
                uints.push_back(255);
            }
        }
    } else if (bitmap.pixel_mode == FT_PIXEL_MODE_GRAY) {
        for (size_t bitIdx = 0; bitIdx < pixelCount; bitIdx += 1) {
            // Convert 8 bits into 3 uints
            // Red
            uints.push_back((uint8_t)bitmapBuffer[bitIdx]);
            // Green
            uints.push_back(uints.back());
            // Blue
            uints.push_back(uints.back());
            // Alpha
            uints.push_back(255);
        }

        for (unsigned int row = 0; row < bitmap.rows; row++) {
            for (unsigned int column = 0; column < bitmap.width; column++) {
                if (bitmapBuffer[row * bitmap.width + column] > 0) {
                    printf("D");
                } else {
                    printf(" ");
                }
            }

            printf("\n");
        }
    }
}
