#include "CommandListVK.hpp"

#include <Engine/Rendering/APIAbstractions/Vulkan/BufferVK.hpp>
#include <Engine/Rendering/APIAbstractions/Vulkan/DeviceVK.hpp>
#include <Engine/Rendering/APIAbstractions/Vulkan/TextureVK.hpp>

CommandListVK::CommandListVK(VkCommandBuffer commandBuffer, VkCommandPool commandPool, DeviceVK* pDevice)
    :m_CommandBuffer(commandBuffer),
    m_CommandPool(commandPool),
    m_pDevice(pDevice)
{}

CommandListVK::~CommandListVK()
{
    vkFreeCommandBuffers(m_pDevice->getDevice(), m_CommandPool, 1u, &m_CommandBuffer);
}

bool CommandListVK::begin(COMMAND_LIST_USAGE usageFlags, CommandListBeginInfo* pBeginInfo)
{
    VkCommandBufferBeginInfo beginInfo = {};
    beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    beginInfo.flags = convertUsageFlags(usageFlags);

    VkCommandBufferInheritanceInfo inheritanceInfo = {};
    if (pBeginInfo) {
        inheritanceInfo = convertBeginInfo(pBeginInfo);
        beginInfo.pInheritanceInfo = &inheritanceInfo;
    }

    if (vkBeginCommandBuffer(m_CommandBuffer, &beginInfo) != VK_SUCCESS) {
        LOG_WARNING("Failed to begin command list");
        return false;
    }

    return true;
}

bool CommandListVK::reset()
{
    if (vkResetCommandBuffer(m_CommandBuffer, 0) != VK_SUCCESS) {
        LOG_WARNING("Failed to reset command list");
        return false;
    }

    return true;
}

void CommandListVK::copyBufferToTexture(IBuffer* pBuffer, Texture* pTexture, uint32_t width, uint32_t height)
{
    VkBufferImageCopy copyInfo = {};
    copyInfo.bufferOffset       = 0u;
    copyInfo.bufferRowLength    = 0u;
    copyInfo.bufferImageHeight  = 0u;
    copyInfo.imageSubresource.aspectMask        = VK_IMAGE_ASPECT_COLOR_BIT;
    copyInfo.imageSubresource.mipLevel          = 0u;
    copyInfo.imageSubresource.baseArrayLayer    = 0u;
    copyInfo.imageSubresource.layerCount        = 0u;
    copyInfo.imageOffset    = {0u, 0u, 0u};
    copyInfo.imageExtent    = {width, height, 1u};

    TextureVK* pTextureVK = reinterpret_cast<TextureVK*>(pTexture);

    VkBuffer srcBuffer          = reinterpret_cast<BufferVK*>(pBuffer)->getBuffer();
    VkImage dstImage            = pTextureVK->getImage();

    vkCmdCopyBufferToImage(m_CommandBuffer, srcBuffer, dstImage, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1u, &copyInfo);
}

VkCommandBufferUsageFlags CommandListVK::convertUsageFlags(COMMAND_LIST_USAGE usageFlags)
{
    return
        HAS_FLAG(usageFlags, COMMAND_LIST_USAGE::ONE_TIME_SUBMIT) * VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT |
        HAS_FLAG(usageFlags, COMMAND_LIST_USAGE::WITHIN_RENDER_PASS) * VK_COMMAND_BUFFER_USAGE_RENDER_PASS_CONTINUE_BIT |
        HAS_FLAG(usageFlags, COMMAND_LIST_USAGE::SIMULTANEOUS_USE) * VK_COMMAND_BUFFER_USAGE_SIMULTANEOUS_USE_BIT;
}

VkCommandBufferInheritanceInfo CommandListVK::convertBeginInfo(CommandListBeginInfo* pBeginInfo)
{
    VkCommandBufferInheritanceInfo inheritanceInfo = {};
    inheritanceInfo.sType       = VK_STRUCTURE_TYPE_COMMAND_BUFFER_INHERITANCE_INFO;
    if (pBeginInfo->pRenderPass)  {
        inheritanceInfo.renderPass = VK_NULL_HANDLE; // TODO: Implement RenderPassVK
    }
    inheritanceInfo.subpass     = pBeginInfo->Subpass;
    if (pBeginInfo->pFramebuffer) {
        inheritanceInfo.framebuffer = VK_NULL_HANDLE; // TODO: Implement FramebufferVK
    }

    return inheritanceInfo;
}
