#pragma once

#include <Engine/ECS/ComponentHandler.hpp>

#include <unordered_map>

class Device;
class Texture;

class TextureLoader : public ComponentHandler
{
public:
    TextureLoader(ECSCore* pECS, Device* pDevice);
    ~TextureLoader() = default;

    virtual bool initHandler() override;

    std::shared_ptr<Texture> loadTexture(const std::string& filePath);

private:
    std::unordered_map<std::string, std::weak_ptr<Texture>> m_Textures;

    Device* m_pDevice;
};
