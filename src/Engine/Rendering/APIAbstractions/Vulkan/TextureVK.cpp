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
        LOG_WARNINGF("Failed to load texture from file: %s", filePath.c_str());
        return nullptr;
    }

    InitialData initialData = {};
    initialData.pData   = pPixelData;
    initialData.RowSize = 4u * (uint32_t)width;

    TextureInfo textureInfo = {};
    textureInfo.Dimensions      = { (uint32_t)width, (uint32_t)height };
    textureInfo.Usage           = TEXTURE_USAGE::SAMPLED | TEXTURE_USAGE::TRANSFER_DST;
    textureInfo.Layout          = TEXTURE_LAYOUT::SHADER_READ_ONLY;
    textureInfo.Format          = RESOURCE_FORMAT::R8G8B8A8_UNORM;
    textureInfo.pInitialData    = &initialData;

    TextureVK* pTexture = create(textureInfo, pDevice);

    stbi_image_free(pPixelData);

    return pTexture;
}

TextureVK* TextureVK::create(const TextureInfo& textureInfo, DeviceVK* pDevice)
{
    TextureInfoVK textureInfoVK = convertTextureInfo(textureInfo);

    VkImage image = VK_NULL_HANDLE;
    VmaAllocation allocation = {};
    if (!createImage(image, allocation, textureInfoVK, pDevice)) {
        return nullptr;
    }

    VkDevice device = pDevice->getDevice();
    VkImageView imageView = createImageView(image, textureInfoVK, device);
    if (imageView == VK_NULL_HANDLE) {
        return nullptr;
    }

    std::unique_ptr<TextureVK> pTexture(DBG_NEW TextureVK(textureInfo.Dimensions, textureInfo.Format, pDevice, image, imageView, allocation));

    // Create temporary command list for layout conversion, and optionally staging buffer copy
    PooledResource<ICommandPool> commandPoolTemp = pDevice->acquireTempCommandPoolGraphics();
    CommandPoolVK* pCommandPoolVK = reinterpret_cast<CommandPoolVK*>(commandPoolTemp.get());

    ICommandList* pCommandList = nullptr;
    if (!pCommandPoolVK->allocateCommandLists(&pCommandList, 1u, COMMAND_LIST_LEVEL::PRIMARY)) {
        LOG_WARNING("Failed to create temporary command list during texture creation");
        return nullptr;
    }

    CommandListVK* pCommandListVK = reinterpret_cast<CommandListVK*>(pCommandList);
    if (!pCommandListVK->begin(COMMAND_LIST_USAGE::ONE_TIME_SUBMIT, nullptr)) {
        return nullptr;
    }

    std::unique_ptr<BufferVK> pStagingBuffer(nullptr);
    if (!textureInfoVK.pInitialData) {
        TextureLayoutConversionInfo conversionInfo = {
            .CommandBuffer  = pCommandListVK->getCommandBuffer(),
            .Image          = image,
            .AspectMask     = textureInfoVK.AspectMask,
            .SrcLayout      = VK_IMAGE_LAYOUT_UNDEFINED,
            .DstLayout      = textureInfoVK.Layout,
            .SrcStage       = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT,
            .DstStage       = VK_PIPELINE_STAGE_ALL_COMMANDS_BIT
        };

        if (!convertTextureLayout(conversionInfo)) {
            LOG_WARNING("Failed to convert image to its desired layout");
            return nullptr;
        }
    } else {
        pStagingBuffer.reset(createStagingBuffer(textureInfoVK, textureInfo.Format, pDevice));
        if (!pStagingBuffer) {
            LOG_WARNING("Failed to create staging buffer during texture creation");
            return nullptr;
        }

        if (!setInitialData(pTexture.get(), textureInfoVK, pStagingBuffer.get(), pCommandListVK)) {
           return nullptr;
        }
    }

    if (!submitTempCommandList(pCommandListVK, commandPoolTemp, pStagingBuffer.get(), pDevice)) {
        return nullptr;
    }

    pStagingBuffer.release();
    return pTexture.release();
}

TextureVK::TextureVK(const glm::uvec2& dimensions, RESOURCE_FORMAT format, DeviceVK* pDevice, VkImage image, VkImageView imageView, VmaAllocation allocation)
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

    if (m_Image != VK_NULL_HANDLE) {
        vkDestroyImage(device, m_Image, nullptr);
    }

    if (m_Allocation != VK_NULL_HANDLE) {
        vmaFreeMemory(m_pDevice->getVulkanAllocator(), m_Allocation);
    }
}

bool TextureVK::convertTextureLayout(VkCommandBuffer commandBuffer, TEXTURE_LAYOUT srcLayout, TEXTURE_LAYOUT dstLayout, PIPELINE_STAGE srcStage, PIPELINE_STAGE dstStage)
{
    VkImageLayout srcLayoutVK = convertImageLayoutFlag(srcLayout);

    TextureLayoutConversionInfo conversionInfo = {};
    conversionInfo.CommandBuffer    = commandBuffer;
    conversionInfo.Image            = m_Image;
    conversionInfo.AspectMask       = layoutToAspectMask(srcLayoutVK);
    conversionInfo.SrcLayout        = convertImageLayoutFlag(srcLayout);
    conversionInfo.DstLayout        = convertImageLayoutFlag(dstLayout);
    conversionInfo.SrcStage         = convertPipelineStageFlags(srcStage);
    conversionInfo.DstStage         = convertPipelineStageFlags(dstStage);

    return convertTextureLayout(conversionInfo);
}

bool TextureVK::setInitialData(TextureVK* pTexture, const TextureInfoVK& textureInfo, BufferVK* pStagingBuffer, CommandListVK* pCommandList)
{
    TextureLayoutConversionInfo conversionInfo = {
        .CommandBuffer  = pCommandList->getCommandBuffer(),
        .Image          = pTexture->getImage(),
        .AspectMask     = textureInfo.AspectMask,
        .SrcLayout      = VK_IMAGE_LAYOUT_UNDEFINED,
        .DstLayout      = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
        .SrcStage       = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT,
        .DstStage       = VK_PIPELINE_STAGE_TRANSFER_BIT
    };

    if (!convertTextureLayout(conversionInfo)) {
        LOG_WARNING("Failed to convert image for a transfer from staging buffer");
        return false;
    }

    pCommandList->copyBufferToTexture(pStagingBuffer, pTexture, textureInfo.Dimensions.x, textureInfo.Dimensions.y);

    conversionInfo.SrcLayout    = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
    conversionInfo.DstLayout    = textureInfo.Layout;
    conversionInfo.SrcStage     = VK_PIPELINE_STAGE_TRANSFER_BIT;
    conversionInfo.DstStage     = VK_PIPELINE_STAGE_ALL_COMMANDS_BIT;

    if (!convertTextureLayout(conversionInfo)) {
        LOG_WARNING("Failed to convert image to its desired layout");
        return false;
    }

    return true;
}

BufferVK* TextureVK::createStagingBuffer(const TextureInfoVK& textureInfo, RESOURCE_FORMAT format, DeviceVK* pDevice)
{
    BufferInfo bufferInfo = {};
    bufferInfo.ByteSize     = size_t(textureInfo.Dimensions.x) * textureInfo.Dimensions.y * getFormatSize(format);
    bufferInfo.pData        = textureInfo.pInitialData->pData;
    bufferInfo.CPUAccess    = BUFFER_DATA_ACCESS::WRITE;
    bufferInfo.GPUAccess    = BUFFER_DATA_ACCESS::READ;
    bufferInfo.Usage        = BUFFER_USAGE::STAGING_BUFFER;
    bufferInfo.QueueFamilyIndices   = { pDevice->getQueueFamilyIndices().Graphics };

    return pDevice->createBuffer(bufferInfo, nullptr);
}

bool TextureVK::submitTempCommandList(CommandListVK* pCommandList, PooledResource<ICommandPool>& tempCommandPool, BufferVK* pStagingBuffer, DeviceVK* pDevice)
{
    if (!pCommandList->end()) {
        return false;
    }

    FenceVK* pFence = pDevice->createFence(false);
    if (!pFence) {
        return false;
    }

    if (!pDevice->graphicsQueueSubmit(pCommandList, pFence, nullptr)) {
        return false;
    }

    // Have a separate thread delete the temporary command list and fence when the commands have finished
    std::function<void()> commandsFinishedFnc = [pFence, pDevice, pStagingBuffer, pCommandList, tempCommandPool]() mutable {
        IFence* pIFence = pFence;
        pDevice->waitForFences(&pIFence, 1u, false, (uint64_t)std::pow(10u, 10u)); // Wait a maximum of 10 seconds
        delete pFence;
        delete pStagingBuffer;
        delete pCommandList;
        tempCommandPool.release();
    };

    std::thread destructorThread(commandsFinishedFnc);
    destructorThread.detach();

    return true;
}

bool TextureVK::createImage(VkImage& image, VmaAllocation& allocation, const TextureInfoVK& textureInfo, DeviceVK* pDevice)
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
    imageInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    imageInfo.usage         = textureInfo.Usage;
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

    if (vmaBindImageMemory(allocator, allocation, image) != VK_SUCCESS) {
        LOG_WARNING("Failed to bind memory for image");
        return false;
    }

    return true;
}

VkImageView TextureVK::createImageView(VkImage image, const TextureInfoVK& textureInfo, VkDevice device)
{
    VkImageViewCreateInfo viewInfo = {};
    viewInfo.sType      = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
    viewInfo.image      = image;
    viewInfo.viewType   = VK_IMAGE_VIEW_TYPE_2D;
    viewInfo.format     = textureInfo.Format;
    viewInfo.subresourceRange.aspectMask        = textureInfo.AspectMask;
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

bool TextureVK::convertTextureLayout(const TextureLayoutConversionInfo& conversionInfo)
{
    VkImageMemoryBarrier barrierInfo = {};
    barrierInfo.sType               = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
    barrierInfo.srcAccessMask       = layoutToAccessMask(conversionInfo.SrcLayout);
    barrierInfo.dstAccessMask       = layoutToAccessMask(conversionInfo.DstLayout);
    barrierInfo.oldLayout           = conversionInfo.SrcLayout;
    barrierInfo.newLayout           = conversionInfo.DstLayout;
    barrierInfo.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    barrierInfo.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    barrierInfo.image               = conversionInfo.Image;
    barrierInfo.subresourceRange.aspectMask     = conversionInfo.AspectMask;
    barrierInfo.subresourceRange.baseMipLevel   = 0u;
    barrierInfo.subresourceRange.levelCount     = 1u;
    barrierInfo.subresourceRange.baseArrayLayer = 0u;
    barrierInfo.subresourceRange.layerCount     = 1u;

    vkCmdPipelineBarrier(
        conversionInfo.CommandBuffer,
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
        case VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_OPTIMAL:
            return VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
        case VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL:
            return VK_ACCESS_TRANSFER_READ_BIT;
        case VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL:
            return VK_ACCESS_TRANSFER_WRITE_BIT;
        case VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL:
            return VK_ACCESS_SHADER_READ_BIT;
        default:
            LOG_ERRORF("Unknown image layout flag: %d", (uint32_t)layout);
            return 0u;
	}
}

VkImageAspectFlags TextureVK::layoutToAspectMask(VkImageLayout layout)
{
    return layout == VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_OPTIMAL || layout == VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL ?
        VK_IMAGE_ASPECT_DEPTH_BIT : VK_IMAGE_ASPECT_COLOR_BIT;
}

VkImageUsageFlags TextureVK::convertUsageMask(TEXTURE_USAGE usage)
{
    return
        HAS_FLAG(usage, TEXTURE_USAGE::TRANSFER_SRC)    * VK_IMAGE_USAGE_TRANSFER_SRC_BIT       |
        HAS_FLAG(usage, TEXTURE_USAGE::TRANSFER_DST)    * VK_IMAGE_USAGE_TRANSFER_DST_BIT       |
        HAS_FLAG(usage, TEXTURE_USAGE::SAMPLED)         * VK_IMAGE_USAGE_SAMPLED_BIT            |
        HAS_FLAG(usage, TEXTURE_USAGE::STORAGE)         * VK_IMAGE_USAGE_STORAGE_BIT            |
        HAS_FLAG(usage, TEXTURE_USAGE::RENDER_TARGET)   * VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT   |
        HAS_FLAG(usage, TEXTURE_USAGE::DEPTH_STENCIL)   * VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT;
}

TextureInfoVK TextureVK::convertTextureInfo(const TextureInfo& textureInfo)
{
    TextureInfoVK textureInfoVK = {};
    textureInfoVK.Dimensions    = textureInfo.Dimensions;
    textureInfoVK.Layout        = convertImageLayoutFlag(textureInfo.Layout);
    textureInfoVK.Usage         = convertUsageMask(textureInfo.Usage) | ((textureInfo.pInitialData != nullptr) * VK_IMAGE_USAGE_TRANSFER_DST_BIT);
    textureInfoVK.Format        = convertFormatToVK(textureInfo.Format);
    textureInfoVK.AspectMask    = layoutToAspectMask(textureInfoVK.Layout);
    textureInfoVK.pInitialData  = textureInfo.pInitialData;

    return textureInfoVK;
}
