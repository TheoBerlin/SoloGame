#pragma once

#include <Engine/Rendering/AssetContainers/AssetResources.hpp>
#include <unordered_map>

class TextureLoader
{
public:
    ~TextureLoader();

    static Texture loadTexture(const std::string& filePath);

    static void deleteAllTextures();

private:
    static std::unordered_map<std::string, Texture> textures;
};
