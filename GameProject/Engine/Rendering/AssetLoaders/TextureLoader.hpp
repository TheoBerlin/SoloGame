#pragma once

#include <Engine/ECS/ComponentHandler.hpp>
#include <Engine/Rendering/AssetContainers/Texture.hpp>
#include <d3d11.h>
#include <unordered_map>

class TextureLoader : public ComponentHandler
{
public:
    TextureLoader(ECSCore* pECS, ID3D11Device* pDevice);
    ~TextureLoader();

    virtual bool init() override;

    TextureReference loadTexture(const std::string& filePath);

    void deleteAllTextures();

private:
    std::unordered_map<std::string, Texture*> m_Textures;

    ID3D11Device* m_pDevice;
};
