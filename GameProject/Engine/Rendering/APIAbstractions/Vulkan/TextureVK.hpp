#pragma once

#define NOMINMAX
#include <vma/vk_mem_alloc.h>
#include <vulkan/vulkan.h>

class BufferVK;
class DeviceVK;

// Mimics TextureInfo, but has converted API-specific flags
struct TextureInfoVK {
    glm::uvec2 Dimensions;
    VkImageLayout Layout;
    VkFormat Format;
    InitialData* pInitialData;  // Optional
};

class TextureVK : public Texture
{
public:
    static TextureVK* createFromFile(const std::string& filePath, DeviceVK* pDevice);
    static TextureVK* create(const TextureInfo& textureInfo, DeviceVK* pDevice);

public:
    TextureVK(const glm::uvec2 dimensions, RESOURCE_FORMAT format, DeviceVK* pDevice, VkImage image, VkImageView imageView, VmaAllocation allocation);
    ~TextureVK();

    bool convertTextureLayout(VkCommandBuffer commandBuffer, TEXTURE_LAYOUT oldLayout, TEXTURE_LAYOUT newLayout, PIPELINE_STAGE srcStage, PIPELINE_STAGE dstStage);

    inline VkImage getImage()           { return m_Image; }
    inline VkImageView getImageView()   { return m_ImageView; }

private:
    static bool setInitialData(TextureVK* pTexture, const TextureInfoVK& textureInfo, DeviceVK* pDevice);
    static BufferVK* createStagingBuffer(const TextureInfoVK& textureInfo, DeviceVK* pDevice);
    // Allocates memory for the image and creates image handle
    static bool createImage(VkImage& image, VmaAllocation& allocation, const TextureInfoVK& textureInfo, VkImageLayout initialLayout, DeviceVK* pDevice);
    static VkImageView createImageView(VkImage image, VkFormat format, VkDevice device);
    static bool convertTextureLayout(VkCommandBuffer commandBuffer, VkImage image, VkImageLayout oldLayout, VkImageLayout newLayout, VkPipelineStageFlags srcStage, VkPipelineStageFlags dstStage);
    static VkAccessFlags layoutToAccessMask(VkImageLayout layout);
    static TextureInfoVK convertTextureInfo(const TextureInfo& textureInfo);

private:
    DeviceVK* m_pDevice;

    VkImage m_Image;
    VkImageView m_ImageView;

    VmaAllocation m_Allocation;
};
