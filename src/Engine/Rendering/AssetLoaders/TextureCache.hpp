#pragma once

#include <Engine/ECS/ComponentHandler.hpp>

#include <unordered_map>

class Device;
class Texture;

class TextureCache : public ComponentHandler
{
public:
    TextureCache(ECSCore* pECS, Device* pDevice);
    ~TextureCache() = default;

    virtual bool initHandler() override;

    std::shared_ptr<Texture> loadTexture(const std::string& filePath);

private:
    std::unordered_map<std::string, std::weak_ptr<Texture>> m_Textures;

    Device* m_pDevice;
};
