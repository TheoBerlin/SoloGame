#pragma once

#include <Engine/ECS/ComponentHandler.hpp>
#include <Engine/Rendering/AssetContainers/Texture.hpp>
#include <d3d11.h>
#include <unordered_map>

class TextureLoader : public ComponentHandler
{
public:
    TextureLoader(SystemSubscriber* sysSubscriber, ID3D11Device* device);
    ~TextureLoader();

    TextureReference loadTexture(const std::string& filePath);

    void deleteAllTextures();

private:
    std::unordered_map<std::string, Texture*> textures;

    ID3D11Device* device;
};
