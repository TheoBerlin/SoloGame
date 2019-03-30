#pragma once

#include <Engine/Rendering/AssetContainers/Texture.hpp>
#include <d3d11.h>
#include <unordered_map>

class TextureLoader
{
public:
    ~TextureLoader();

    static void setDevice(ID3D11Device* device);

    static ID3D11ShaderResourceView* loadTexture(const std::string& filePath);

    static void deleteAllTextures();

private:
    static std::unordered_map<std::string, ID3D11ShaderResourceView*> textures;

    static ID3D11Device* device;
};
