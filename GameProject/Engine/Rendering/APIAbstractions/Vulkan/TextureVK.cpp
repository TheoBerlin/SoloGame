#include "TextureVK.hpp"

#define STB_IMAGE_IMPLEMENTATION
#include <stb/stb_image.h>

TextureVK* TextureVK::createFromFile(const std::string& filePath)
{
    int width = 0, height = 0, texChannels = 0;
    stbi_uc* pPixelData = stbi_load(filePath.c_str(), &width, &height, &texChannels, STBI_rgb_alpha);
    if (!pPixelData) {
        LOG_WARNING("Failed to load texture from file: %s", filePath.c_str());
        return nullptr;
    }

    VkDeviceSize texSize = width * height * 4;
    // TODO
    return nullptr;

}

TextureVK* TextureVK::create(const TextureInfo& textureInfo)
{
    // TODO
    return nullptr;
}
