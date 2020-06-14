#pragma once

#define NOMINMAX
#include <vma/vk_mem_alloc.h>
#include <vulkan/vulkan.h>

class BufferVK;
class DeviceVK;

class TextureVK : Texture
{
public:
    static TextureVK* createFromFile(const std::string& filePath, DeviceVK* pDevice);
    static TextureVK* create(const TextureInfo& textureInfo);

public:
    TextureVK(const glm::uvec2 dimensions, RESOURCE_FORMAT format, DeviceVK* pDevice, VkImage image, VkImageView imageView, VmaAllocation allocation);
    ~TextureVK();

    bool convertTextureLayout(VkCommandBuffer commandBuffer, TEXTURE_LAYOUT oldLayout, TEXTURE_LAYOUT newLayout, PIPELINE_STAGE srcStage, PIPELINE_STAGE dstStage);

    inline VkImage getImage() { return m_Image; }

private:
    static BufferVK* createStagingBuffer(const unsigned char* pPixelData, VkDeviceSize textureSize, DeviceVK* pDevice);
    // Allocates memory for the image and creates image handle
    static bool createImage(VkImage& image, VmaAllocation& allocation, uint32_t width, uint32_t height, DeviceVK* pDevice);
    static VkImageView createImageView(VkImage image, VkFormat format, VkDevice device);
    static bool convertTextureLayout(VkCommandBuffer commandBuffer, VkImage image, VkImageLayout oldLayout, VkImageLayout newLayout, VkPipelineStageFlags srcStage, VkPipelineStageFlags dstStage);
    static VkAccessFlags layoutToAccessMask(VkImageLayout layout);

private:
    DeviceVK* m_pDevice;

    VkImage m_Image;
    VkImageView m_ImageView;

    VmaAllocation m_Allocation;
};
