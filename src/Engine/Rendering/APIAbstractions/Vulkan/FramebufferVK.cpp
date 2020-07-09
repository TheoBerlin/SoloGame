#include "FramebufferVK.hpp"

#include <Engine/Rendering/APIAbstractions/Vulkan/DeviceVK.hpp>
#include <Engine/Rendering/APIAbstractions/Vulkan/RenderPassVK.hpp>
#include <Engine/Rendering/APIAbstractions/Vulkan/TextureVK.hpp>

FramebufferVK* FramebufferVK::create(const FramebufferInfo& framebufferInfo, DeviceVK* pDevice)
{
    std::vector<VkImageView> attachments;
    attachments.reserve(framebufferInfo.Attachments.size());
    for (Texture* pTexture : framebufferInfo.Attachments) {
        attachments.push_back(reinterpret_cast<TextureVK*>(pTexture)->getImageView());
    }

    VkFramebufferCreateInfo framebufferInfoVK = {};
    framebufferInfoVK.sType             = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
    framebufferInfoVK.renderPass        = reinterpret_cast<RenderPassVK*>(framebufferInfo.pRenderPass)->getRenderPass();
    framebufferInfoVK.attachmentCount   = (uint32_t)attachments.size();
    framebufferInfoVK.pAttachments      = attachments.data();
    framebufferInfoVK.width             = (uint32_t)framebufferInfo.Dimensions.x;
    framebufferInfoVK.height            = (uint32_t)framebufferInfo.Dimensions.y;
    framebufferInfoVK.layers            = 1u;

    VkFramebuffer framebuffer = VK_NULL_HANDLE;
    if (vkCreateFramebuffer(pDevice->getDevice(), &framebufferInfoVK, nullptr, &framebuffer) != VK_SUCCESS) {
        LOG_ERROR("Failed to create framebuffer");
        return nullptr;
    }

    return DBG_NEW FramebufferVK(framebuffer, framebufferInfo.Dimensions, pDevice);
}

FramebufferVK::FramebufferVK(VkFramebuffer framebuffer, const glm::uvec2& dimensions, DeviceVK* pDevice)
    :Framebuffer(dimensions),
    m_Framebuffer(framebuffer),
    m_pDevice(pDevice)
{}

FramebufferVK::~FramebufferVK()
{
    vkDestroyFramebuffer(m_pDevice->getDevice(), m_Framebuffer, nullptr);
}
