#pragma once

#define NOMINMAX
#include <vma/vk_mem_alloc.h>
#include <vulkan/vulkan.h>

class BufferVK;
class CommandListVK;
class DeviceVK;

struct TextureInfoVK {
    glm::uvec2 Dimensions;
    VkImageLayout Layout;
    VkImageUsageFlags Usage;
    VkFormat Format;
    VkImageAspectFlags AspectMask;
    InitialData* pInitialData;  // Optional
};

struct TextureLayoutConversionInfo {
    VkCommandBuffer CommandBuffer;
    VkImage Image;
    VkImageAspectFlags AspectMask;
    VkImageLayout SrcLayout, DstLayout;
    VkPipelineStageFlags SrcStage, DstStage;
};

class TextureVK : public Texture
{
public:
    static TextureVK* createFromFile(const std::string& filePath, DeviceVK* pDevice);
    static TextureVK* create(const TextureInfo& textureInfo, DeviceVK* pDevice);

public:
    // TODO: Pack parameter list into struct
    TextureVK(const glm::uvec2& dimensions, RESOURCE_FORMAT format, DeviceVK* pDevice, VkImage image, VkImageView imageView, VmaAllocation allocation);
    ~TextureVK();

    bool convertTextureLayout(VkCommandBuffer commandBuffer, TEXTURE_LAYOUT oldLayout, TEXTURE_LAYOUT newLayout, PIPELINE_STAGE srcStage, PIPELINE_STAGE dstStage);

    inline VkImage getImage()               { return m_Image; }
    inline VkImageView getImageView() const { return m_ImageView; }

private:
    static bool setInitialData(TextureVK* pTexture, const TextureInfoVK& textureInfo, BufferVK* pStagingBuffer, CommandListVK* pCommandList);
    static BufferVK* createStagingBuffer(const TextureInfoVK& textureInfo, RESOURCE_FORMAT format, DeviceVK* pDevice);
    static bool submitTempCommandList(CommandListVK* pCommandList, PooledResource<ICommandPool>& tempCommandPool, BufferVK* pStagingBuffer, DeviceVK* pDevice);
    // Allocates memory for the image and creates image handle
    static bool createImage(VkImage& image, VmaAllocation& allocation, const TextureInfoVK& textureInfo, DeviceVK* pDevice);
    static VkImageView createImageView(VkImage image, const TextureInfoVK& textureInfo, VkDevice device);
    static bool convertTextureLayout(const TextureLayoutConversionInfo& conversionInfo);
    static VkAccessFlags layoutToAccessMask(VkImageLayout layout);
    static VkImageAspectFlags layoutToAspectMask(VkImageLayout layout);
    static VkImageUsageFlags convertUsageMask(TEXTURE_USAGE usage);
    static TextureInfoVK convertTextureInfo(const TextureInfo& textureInfo);

private:
    DeviceVK* m_pDevice;

    VkImage m_Image;
    VkImageView m_ImageView;

    VmaAllocation m_Allocation;
};
