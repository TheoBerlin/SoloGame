#pragma once

#include <vulkan/vulkan.h>

class TextureVK : Texture
{
public:
    static TextureVK* createFromFile(const std::string& filePath);
    static TextureVK* create(const TextureInfo& textureInfo);

public:
    TextureVK();
    ~TextureVK();

private:
    VkImage m_Image;
    VkImageView m_ImageView;
};
