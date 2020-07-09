#pragma once

#include <Engine/Rendering/APIAbstractions/RenderPass.hpp>

#include <vulkan/vulkan.h>

class DeviceVK;

class RenderPassVK : public IRenderPass
{
public:
    static RenderPassVK* create(const RenderPassInfo& renderPassInfo, DeviceVK* pDevice);

public:
    RenderPassVK(VkRenderPass renderPass, DeviceVK* pDevice);
    ~RenderPassVK();

    void begin(const RenderPassBeginInfo& beginInfo, VkCommandBuffer commandBuffer);

    inline VkRenderPass getRenderPass() { return m_RenderPass; }

private:
    static VkAttachmentDescription convertAttachmentInfo(const AttachmentInfo& attachmentInfo);
    static VkSubpassDescription convertSubpassInfo(const SubpassInfo& subpassInfo, std::vector<VkAttachmentReference>& attachmentReferencesColor, VkAttachmentReference& attachmentReferenceDepth);
    static VkSubpassDependency convertSubpassDependency(const SubpassDependency& subpassDependency);

    static VkSampleCountFlagBits convertSampleCount(uint32_t sampleCount);
    static VkAttachmentLoadOp convertAttachmentLoadOp(ATTACHMENT_LOAD_OP loadOp);
    static VkAttachmentStoreOp convertAttachmentStoreOp(ATTACHMENT_STORE_OP storeOp);
    static VkAttachmentReference convertAttachmentReference(const AttachmentReference& attachmentReference);

private:
    VkRenderPass m_RenderPass;
    DeviceVK* m_pDevice;
};
