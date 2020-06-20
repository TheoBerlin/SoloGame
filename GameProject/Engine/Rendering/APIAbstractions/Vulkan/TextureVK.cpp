#include "TextureVK.hpp"

#include <Engine/Rendering/APIAbstractions/Vulkan/BufferVK.hpp>
#include <Engine/Rendering/APIAbstractions/Vulkan/CommandListVK.hpp>
#include <Engine/Rendering/APIAbstractions/Vulkan/CommandPoolVK.hpp>
#include <Engine/Rendering/APIAbstractions/Vulkan/DeviceVK.hpp>
#include <Engine/Rendering/APIAbstractions/Vulkan/FenceVK.hpp>
#include <Engine/Rendering/APIAbstractions/Vulkan/GeneralResourcesVK.hpp>

#define STB_IMAGE_IMPLEMENTATION
#include <stb/stb_image.h>

#include <memory>
#include <thread>

TextureVK* TextureVK::createFromFile(const std::string& filePath, DeviceVK* pDevice)
{
    int width = 0, height = 0, texChannels = 0;
    stbi_uc* pPixelData = stbi_load(filePath.c_str(), &width, &height, &texChannels, STBI_rgb_alpha);
    if (!pPixelData) {
        LOG_WARNING("Failed to load texture from file: %s", filePath.c_str());
        return nullptr;
    }

    // Let TextureVK::create handle the rest of the creation steps
    InitialData initialData = {};
    initialData.pData   = pPixelData;
    initialData.RowSize = 4u * (uint32_t)width;

    TextureInfo textureInfo = {};
    textureInfo.Dimensions      = { (uint32_t)width, (uint32_t)height };
    textureInfo.InitialLayout   = TEXTURE_LAYOUT::SHADER_READ_ONLY;
    textureInfo.Format          = RESOURCE_FORMAT::R8G8B8A8_SRGB;
    textureInfo.pInitialData    = &initialData;

    TextureVK* pTexture = create(textureInfo, pDevice);

    stbi_image_free(pPixelData);

    return pTexture;
}

TextureVK* TextureVK::create(const TextureInfo& textureInfo, DeviceVK* pDevice)
{
    TextureInfoVK textureInfoVK = convertTextureInfo(textureInfo);
    VkImageLayout initialLayout = textureInfo.pInitialData ? VK_IMAGE_LAYOUT_UNDEFINED : textureInfoVK.Layout;

    VkImage image = VK_NULL_HANDLE;
    VmaAllocation allocation = {};
    if (!createImage(image, allocation, textureInfoVK, initialLayout, pDevice)) {
        return nullptr;
    }

    VkDevice device = pDevice->getDevice();
    VkImageView imageView = createImageView(image, textureInfoVK.Format, device);
    if (imageView == VK_NULL_HANDLE) {
        return nullptr;
    }

    std::unique_ptr<TextureVK> pTexture(DBG_NEW TextureVK(textureInfo.Dimensions, textureInfo.Format, pDevice, image, imageView, allocation));

    if (textureInfoVK.pInitialData && !setInitialData(pTexture.get(), textureInfoVK, pDevice)) {
        return nullptr;
    }

    return pTexture.release();
}

TextureVK::TextureVK(const glm::uvec2 dimensions, RESOURCE_FORMAT format, DeviceVK* pDevice, VkImage image, VkImageView imageView, VmaAllocation allocation)
    :Texture(dimensions, format),
    m_pDevice(pDevice),
    m_Image(image),
    m_ImageView(imageView),
    m_Allocation(allocation)
{}

TextureVK::~TextureVK()
{
    VkDevice device = m_pDevice->getDevice();

    vkDestroyImageView(device, m_ImageView, nullptr);
    vkDestroyImage(device, m_Image, nullptr);
    vmaFreeMemory(m_pDevice->getVulkanAllocator(), m_Allocation);
}

bool TextureVK::setInitialData(TextureVK* pTexture, const TextureInfoVK& textureInfo, DeviceVK* pDevice)
{
    std::unique_ptr<BufferVK> pStagingBuffer(createStagingBuffer(textureInfo, pDevice));
    if (!pStagingBuffer) {
        LOG_WARNING("Failed to create staging buffer during texture creation");
        return false;
    }

    PooledResource<ICommandPool> commandPoolTemp = pDevice->acquireTempCommandPoolGraphics();
    CommandPoolVK* pCommandPoolVK = reinterpret_cast<CommandPoolVK*>(commandPoolTemp.get());
    ICommandList* pCommandList = nullptr;

    if (!pCommandPoolVK->allocateCommandLists(&pCommandList, 1u, COMMAND_LIST_LEVEL::PRIMARY)) {
        LOG_WARNING("Failed to create temporary command list during texture creation");
        return false;
    }

    CommandListVK* pCommandListVK = reinterpret_cast<CommandListVK*>(pCommandList);
    if (!pCommandListVK->begin(COMMAND_LIST_USAGE::ONE_TIME_SUBMIT, nullptr)) {
        return false;
    }

    VkCommandBuffer commandBuffer = pCommandListVK->getCommandBuffer();
    if (!convertTextureLayout(commandBuffer, pTexture->getImage(),
        VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
        VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT)
    ) {
        LOG_WARNING("Failed to convert image for a transfer from staging buffer");
        return false;
    }

    pCommandListVK->copyBufferToTexture(pStagingBuffer.get(), pTexture, textureInfo.Dimensions.x, textureInfo.Dimensions.y);

    if (!convertTextureLayout(commandBuffer, pTexture->getImage(),
        VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
        VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_ALL_COMMANDS_BIT)
    ) {
        LOG_WARNING("Failed to convert image for shader reading");
        return false;
    }

    if (!pCommandListVK->end()) {
        return false;
    }

    commandPoolTemp.release();

    FenceVK* pFence = pDevice->createFence(false);
    if (!pFence) {
        return false;
    }

    SemaphoreSubmitInfo semaphoreInfo = {};
    if (!pDevice->graphicsQueueSubmit(pCommandList, pFence, semaphoreInfo)) {
        return false;
    }

    // Have a separate thread delete the temporary command list and fence when the commands have finished
    std::function<void()> commandsFinishedFnc = [pFence, pCommandList, pDevice]() {
        IFence* pIFence = pFence;
        pDevice->waitForFences(&pIFence, 1u, false, 0u);
        delete pFence;
        delete pCommandList;
    };

    std::thread destructorThread(commandsFinishedFnc);
    return true;
}

BufferVK* TextureVK::createStagingBuffer(const TextureInfoVK& textureInfo, DeviceVK* pDevice)
{
    BufferInfo bufferInfo = {};
    bufferInfo.ByteSize = size_t(textureInfo.Dimensions.x * textureInfo.Dimensions.y);
    bufferInfo.pData    = textureInfo.pInitialData->pData;
    bufferInfo.CPUAccess    = BUFFER_DATA_ACCESS::WRITE;
    bufferInfo.GPUAccess    = BUFFER_DATA_ACCESS::READ;
    bufferInfo.Usage        = BUFFER_USAGE::STAGING_BUFFER;
    bufferInfo.QueueFamilyIndices   = { pDevice->getQueueFamilyIndices().Graphics };

    return pDevice->createBuffer(bufferInfo, nullptr);
}

bool TextureVK::createImage(VkImage& image, VmaAllocation& allocation, const TextureInfoVK& textureInfo, VkImageLayout initialLayout, DeviceVK* pDevice)
{
    VkImageCreateInfo imageInfo = {};
    imageInfo.sType         = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
    imageInfo.imageType     = VK_IMAGE_TYPE_2D;
    imageInfo.extent.width  = textureInfo.Dimensions.x;
    imageInfo.extent.height = textureInfo.Dimensions.y;
    imageInfo.extent.depth  = 1;
    imageInfo.mipLevels     = 1;
    imageInfo.arrayLayers   = 1;
    imageInfo.format        = textureInfo.Format;
    imageInfo.tiling        = VK_IMAGE_TILING_OPTIMAL;
    imageInfo.initialLayout = initialLayout;
    imageInfo.usage         = VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT;
    imageInfo.samples       = VK_SAMPLE_COUNT_1_BIT;
    imageInfo.sharingMode   = VK_SHARING_MODE_EXCLUSIVE;

    if (vkCreateImage(pDevice->getDevice(), &imageInfo, nullptr, &image) != VK_SUCCESS) {
        LOG_WARNING("Failed to create image");
        return false;
    }

    VmaAllocationCreateInfo allocationInfo = {};
    allocationInfo.usage            = VMA_MEMORY_USAGE_GPU_ONLY;
    allocationInfo.requiredFlags    = VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT;

    VmaAllocator allocator = pDevice->getVulkanAllocator();
    if (vmaAllocateMemoryForImage(allocator, image, &allocationInfo, &allocation, nullptr) != VK_SUCCESS) {
        LOG_WARNING("Failed to allocate memory for image");
        return false;
    }

    return true;
}

VkImageView TextureVK::createImageView(VkImage image, VkFormat format, VkDevice device)
{
    VkImageViewCreateInfo viewInfo = {};
    viewInfo.sType      = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
    viewInfo.image      = image;
    viewInfo.viewType   = VK_IMAGE_VIEW_TYPE_2D;
    viewInfo.format     = format;
    viewInfo.subresourceRange.aspectMask        = VK_IMAGE_ASPECT_COLOR_BIT;
    viewInfo.subresourceRange.baseMipLevel      = 0;
    viewInfo.subresourceRange.levelCount        = 1;
    viewInfo.subresourceRange.baseArrayLayer    = 0;
    viewInfo.subresourceRange.layerCount        = 1;

    VkImageView imageView = VK_NULL_HANDLE;
    if (vkCreateImageView(device, &viewInfo, nullptr, &imageView) != VK_SUCCESS) {
        LOG_WARNING("Failed to create texture image view");
        return VK_NULL_HANDLE;
    }

    return imageView;
}

bool TextureVK::convertTextureLayout(VkCommandBuffer commandBuffer, VkImage image, VkImageLayout srcLayout, VkImageLayout dstLayout, VkPipelineStageFlags srcStage, VkPipelineStageFlags dstStage)
{
    VkImageMemoryBarrier barrierInfo = {};
    barrierInfo.sType               = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
    barrierInfo.srcAccessMask       = layoutToAccessMask(srcLayout);
    barrierInfo.dstAccessMask       = layoutToAccessMask(dstLayout);
    barrierInfo.oldLayout           = srcLayout;
    barrierInfo.newLayout           = dstLayout;
    barrierInfo.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    barrierInfo.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    barrierInfo.image               = image;
    barrierInfo.subresourceRange.aspectMask     = VK_IMAGE_ASPECT_COLOR_BIT;
    barrierInfo.subresourceRange.baseMipLevel   = 0u;
    barrierInfo.subresourceRange.levelCount     = 1u;
    barrierInfo.subresourceRange.baseArrayLayer = 0u;
    barrierInfo.subresourceRange.layerCount     = 1u;

    vkCmdPipelineBarrier(
        commandBuffer,
        VK_PIPELINE_STAGE_ALL_COMMANDS_BIT, VK_PIPELINE_STAGE_ALL_COMMANDS_BIT,
        0,
        0, nullptr,
        0, nullptr,
        1, &barrierInfo
    );

    return true;
}

VkAccessFlags TextureVK::layoutToAccessMask(VkImageLayout layout)
{
    switch (layout) {
        case VK_IMAGE_LAYOUT_UNDEFINED:
        case VK_IMAGE_LAYOUT_GENERAL:
            return 0u;
        case VK_IMAGE_LAYOUT_PREINITIALIZED:
            return VK_ACCESS_HOST_WRITE_BIT;
        case VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL:
            return VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
        case VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL:
            return VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
        case VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL:
            return VK_ACCESS_TRANSFER_READ_BIT;
        case VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL:
            return VK_ACCESS_TRANSFER_WRITE_BIT;
        case VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL:
            return VK_ACCESS_SHADER_READ_BIT;
        default:
            LOG_ERROR("Unknown image layout flag: %d", (uint32_t)layout);
            return 0u;
	}
}

TextureInfoVK TextureVK::convertTextureInfo(const TextureInfo& textureInfo)
{
    TextureInfoVK textureInfoVK = {};
    textureInfoVK.Dimensions    = textureInfo.Dimensions;
    textureInfoVK.Layout        = convertImageLayoutFlag(textureInfo.InitialLayout);
    textureInfoVK.Format        = convertFormatToVK(textureInfo.Format);
    textureInfoVK.pInitialData  = textureInfo.pInitialData;

    return textureInfoVK;
}
