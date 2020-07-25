#include "CommandListVK.hpp"

#include <Engine/Rendering/APIAbstractions/Vulkan/BufferVK.hpp>
#include <Engine/Rendering/APIAbstractions/Vulkan/DescriptorSetVK.hpp>
#include <Engine/Rendering/APIAbstractions/Vulkan/DeviceVK.hpp>
#include <Engine/Rendering/APIAbstractions/Vulkan/FramebufferVK.hpp>
#include <Engine/Rendering/APIAbstractions/Vulkan/PipelineLayoutVK.hpp>
#include <Engine/Rendering/APIAbstractions/Vulkan/PipelineVK.hpp>
#include <Engine/Rendering/APIAbstractions/Vulkan/RenderPassVK.hpp>
#include <Engine/Rendering/APIAbstractions/Vulkan/TextureVK.hpp>

CommandListVK::CommandListVK(VkCommandBuffer commandBuffer, VkCommandPool commandPool, DeviceVK* pDevice)
    :m_CommandBuffer(commandBuffer),
    m_CommandPool(commandPool),
    m_pDevice(pDevice)
{}

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

void CommandListVK::executeSecondaryCommandList(ICommandList* pSecondaryCommandList)
{
    VkCommandBuffer secondaryBuffer = reinterpret_cast<CommandListVK*>(pSecondaryCommandList)->getCommandBuffer();
    vkCmdExecuteCommands(m_CommandBuffer, 1u, &secondaryBuffer);
}

void CommandListVK::beginRenderPass(IRenderPass* pRenderPass, const RenderPassBeginInfo& beginInfo)
{
    reinterpret_cast<RenderPassVK*>(pRenderPass)->begin(beginInfo, m_CommandBuffer);
}

void CommandListVK::nextSubpass(COMMAND_LIST_LEVEL subpassCommandListLevel)
{
    VkSubpassContents subpassContents = subpassCommandListLevel == COMMAND_LIST_LEVEL::PRIMARY ?
        VK_SUBPASS_CONTENTS_INLINE : VK_SUBPASS_CONTENTS_SECONDARY_COMMAND_BUFFERS;

    vkCmdNextSubpass(m_CommandBuffer, subpassContents);
}

void CommandListVK::endRenderPass()
{
    vkCmdEndRenderPass(m_CommandBuffer);
}

void CommandListVK::bindPipeline(IPipeline* pPipeline)
{
    VkPipeline pipeline = reinterpret_cast<PipelineVK*>(pPipeline)->getPipeline();
    vkCmdBindPipeline(m_CommandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline);
}

void CommandListVK::bindDescriptorSet(DescriptorSet* pDescriptorSet, IPipelineLayout* pPipelineLayout, uint32_t setNr)
{
    VkDescriptorSet descriptorSet   = reinterpret_cast<DescriptorSetVK*>(pDescriptorSet)->getDescriptorSet();
    VkPipelineLayout pipelineLayout = reinterpret_cast<PipelineLayoutVK*>(pPipelineLayout)->getPipelineLayout();

    vkCmdBindDescriptorSets(m_CommandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipelineLayout, setNr, 1u, &descriptorSet, 0u, nullptr);
}

void CommandListVK::bindVertexBuffer(uint32_t firstBinding, IBuffer* pBuffer)
{
    VkBuffer buffer = reinterpret_cast<BufferVK*>(pBuffer)->getBuffer();
    VkDeviceSize offset = 0u;
    vkCmdBindVertexBuffers(m_CommandBuffer, firstBinding, 1u, &buffer, &offset);
}

void CommandListVK::bindIndexBuffer(IBuffer* pBuffer)
{
    VkBuffer indexBuffer = reinterpret_cast<BufferVK*>(pBuffer)->getBuffer();
    vkCmdBindIndexBuffer(m_CommandBuffer, indexBuffer, 0u, VK_INDEX_TYPE_UINT32);
}

void CommandListVK::bindViewport(const Viewport* pViewport)
{
    vkCmdSetViewport(m_CommandBuffer, 0u, 1u, (VkViewport*)pViewport);
}

void CommandListVK::bindScissor(const Rectangle2D& scissorRectangle)
{
    vkCmdSetScissor(m_CommandBuffer, 0u, 1u, (VkRect2D*)&scissorRectangle);
}

void CommandListVK::draw(size_t vertexCount)
{
    vkCmdDraw(m_CommandBuffer, (uint32_t)vertexCount, 1u, 0u, 0u);
}

void CommandListVK::drawIndexed(size_t indexCount)
{
    vkCmdDrawIndexed(m_CommandBuffer, (uint32_t)indexCount, 1u, 0u, 0u, 0u);
}

void CommandListVK::convertTextureLayout(TEXTURE_LAYOUT oldLayout, TEXTURE_LAYOUT newLayout, Texture* pTexture, PIPELINE_STAGE srcStage, PIPELINE_STAGE dstStage)
{
    reinterpret_cast<TextureVK*>(pTexture)->convertTextureLayout(m_CommandBuffer, oldLayout, newLayout, srcStage, dstStage);
}

void CommandListVK::copyBuffer(IBuffer* pSrc, IBuffer* pDst, size_t byteSize)
{
    VkBuffer srcBuffer = reinterpret_cast<BufferVK*>(pSrc)->getBuffer();
    VkBuffer dstBuffer = reinterpret_cast<BufferVK*>(pDst)->getBuffer();

    VkBufferCopy copyInfo = {};
    copyInfo.size = (VkDeviceSize)byteSize;

    vkCmdCopyBuffer(m_CommandBuffer, srcBuffer, dstBuffer, 1u, &copyInfo);
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
    copyInfo.imageSubresource.layerCount        = 1u;
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
    inheritanceInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_INHERITANCE_INFO;
    if (pBeginInfo->pRenderPass)  {
        inheritanceInfo.renderPass = reinterpret_cast<RenderPassVK*>(pBeginInfo->pRenderPass)->getRenderPass();
    }
    inheritanceInfo.subpass = pBeginInfo->Subpass;
    if (pBeginInfo->pFramebuffer) {
        inheritanceInfo.framebuffer = reinterpret_cast<FramebufferVK*>(pBeginInfo->pFramebuffer)->getFramebuffer();
    }

    return inheritanceInfo;
}
