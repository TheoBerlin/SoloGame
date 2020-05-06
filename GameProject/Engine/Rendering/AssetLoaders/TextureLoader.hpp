#pragma once

#include <Engine/ECS/ComponentHandler.hpp>

#include <unordered_map>

class IDevice;
class Texture;

class TextureLoader : public ComponentHandler
{
public:
    TextureLoader(ECSCore* pECS, IDevice* pDevice);
    ~TextureLoader() = default;

    virtual bool initHandler() override;

    std::shared_ptr<Texture> loadTexture(const std::string& filePath);

private:
    std::unordered_map<std::string, std::weak_ptr<Texture>> m_Textures;

    IDevice* m_pDevice;
};
