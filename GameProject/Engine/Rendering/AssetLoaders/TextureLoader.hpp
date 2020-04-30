#pragma once

#include <Engine/ECS/ComponentHandler.hpp>
#include <Engine/Rendering/AssetContainers/Texture.hpp>

#include <unordered_map>

class IDevice;

class TextureLoader : public ComponentHandler
{
public:
    TextureLoader(ECSCore* pECS, IDevice* pDevice);
    ~TextureLoader();

    virtual bool initHandler() override;

    TextureReference loadTexture(const std::string& filePath);

    void deleteAllTextures();

private:
    std::unordered_map<std::string, Texture*> m_Textures;

    IDevice* m_pDevice;
};
