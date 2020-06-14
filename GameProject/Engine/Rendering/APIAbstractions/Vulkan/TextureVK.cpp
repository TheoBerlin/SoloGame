#include "TextureVK.hpp"

#include <Engine/Rendering/APIAbstractions/Vulkan/BufferVK.hpp>
#include <Engine/Rendering/APIAbstractions/Vulkan/CommandListVK.hpp>
#include <Engine/Rendering/APIAbstractions/Vulkan/DeviceVK.hpp>
#include <Engine/Rendering/APIAbstractions/Vulkan/FenceVK.hpp>

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

    VkDeviceSize texSize = width * height * 4;

    std::unique_ptr<BufferVK> pStagingBuffer(createStagingBuffer(pPixelData, texSize, pDevice));
    if (!pStagingBuffer) {
        LOG_WARNING("Failed to create staging buffer during texture creation");
        return nullptr;
    }

    stbi_image_free(pPixelData);

    VkImage image = VK_NULL_HANDLE;
    VmaAllocation allocation = {};
    if (!createImage(image, allocation, (uint32_t)width, (uint32_t)height, pDevice)) {
        return nullptr;
    }

    VkDevice device = pDevice->getDevice();
    VkImageView imageView = createImageView(image, VK_FORMAT_R8G8B8A8_SRGB, device);
    if (imageView == VK_NULL_HANDLE) {
        return nullptr;
    }

    // Although the texture object is created here, it still needs its data, and layout conversions
    std::unique_ptr<TextureVK> pTexture(DBG_NEW TextureVK({(unsigned int)width, (unsigned int)height}, RESOURCE_FORMAT::R8G8B8A8_SRGB, pDevice, image, imageView, allocation));

    PooledResource<ICommandPool> commandPoolTemp = pDevice->acquireTempCommandPoolGraphics();
    // TODO: Implement CommandPoolVK
    //CommandPoolVK* pCommandPoolVK = reinterpret_cast<CommandPoolVK*>(commandPoolTemp.get());
    ICommandList* pCommandList = nullptr;

    if (!commandPoolTemp->allocateCommandLists(&pCommandList, 1u, COMMAND_LIST_LEVEL::PRIMARY)) {
        LOG_WARNING("Failed to create temporary command list during texture creation");
        return nullptr;
    }

    CommandListVK* pCommandListVK = reinterpret_cast<CommandListVK*>(pCommandList);
    if (!pCommandListVK->begin(COMMAND_LIST_USAGE::ONE_TIME_SUBMIT, nullptr)) {
        return nullptr;
    }

    VkCommandBuffer commandBuffer = pCommandListVK->getCommandBuffer();
    if (!convertTextureLayout(commandBuffer, image,
        VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
        VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT)
    ) {
        LOG_WARNING("Failed to convert image for a transfer from staging buffer");
        return nullptr;
    }

    pCommandListVK->copyBufferToTexture(pStagingBuffer.get(), pTexture.get(), (uint32_t)width, (uint32_t)height);

    if (!convertTextureLayout(commandBuffer, image,
        VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
        VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_ALL_COMMANDS_BIT)
    ) {
        LOG_WARNING("Failed to convert image for shader reading");
        return nullptr;
    }

    if (!pCommandList->end()) {
        return nullptr;
    }

    commandPoolTemp.release();

    FenceVK* pFence = pDevice->createFence(false);
    if (!pFence) {
        return nullptr;
    }

    SemaphoreSubmitInfo semaphoreInfo = {};
    if (!pDevice->graphicsQueueSubmit(pCommandList, pFence, semaphoreInfo)) {
        return nullptr;
    }

    // Have a separate thread delete the temporary command list and fence when the commands have finished
    std::function<void()> commandsFinishedFnc = [pFence, pCommandList, pDevice]() {
        IFence* pIFence = pFence;
        pDevice->waitForFences(&pIFence, 1u, false, 0u);
        delete pFence;
        delete pCommandList;
    };

    std::thread destructorThread(commandsFinishedFnc);

    return pTexture.release();
}

TextureVK* TextureVK::create(const TextureInfo& textureInfo)
{
    // TODO
    return nullptr;
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

BufferVK* TextureVK::createStagingBuffer(const unsigned char* pPixelData, VkDeviceSize textureSize, DeviceVK* pDevice)
{
    BufferInfo bufferInfo = {};
    bufferInfo.ByteSize = (size_t)textureSize;
    bufferInfo.pData    = pPixelData;
    bufferInfo.CPUAccess    = BUFFER_DATA_ACCESS::WRITE;
    bufferInfo.GPUAccess    = BUFFER_DATA_ACCESS::READ;
    bufferInfo.Usage        = BUFFER_USAGE::STAGING_BUFFER;
    bufferInfo.QueueFamilyIndices   = { pDevice->getQueueFamilyIndices().Graphics };

    return pDevice->createBuffer(bufferInfo, nullptr);
}

bool TextureVK::createImage(VkImage& image, VmaAllocation& allocation, uint32_t width, uint32_t height, DeviceVK* pDevice)
{
    VkImageCreateInfo imageInfo = {};
    imageInfo.sType         = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
    imageInfo.imageType     = VK_IMAGE_TYPE_2D;
    imageInfo.extent.width  = width;
    imageInfo.extent.height = height;
    imageInfo.extent.depth  = 1;
    imageInfo.mipLevels     = 1;
    imageInfo.arrayLayers   = 1;
    imageInfo.format        = VK_FORMAT_R8G8B8A8_SRGB;
    imageInfo.tiling        = VK_IMAGE_TILING_OPTIMAL;
    imageInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
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
