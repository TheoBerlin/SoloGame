#pragma once

#include <unordered_map>

class Device;
class Texture;

class TextureCache
{
public:
    TextureCache(Device* pDevice);
    ~TextureCache() = default;

    std::shared_ptr<Texture> LoadTexture(const std::string& filePath);

private:
    std::unordered_map<std::string, std::weak_ptr<Texture>> m_Textures;

    Device* m_pDevice;
};
