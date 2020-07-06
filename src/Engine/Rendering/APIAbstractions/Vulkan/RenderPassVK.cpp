#include "RenderPassVK.hpp"

#include <Engine/Rendering/APIAbstractions/Vulkan/DeviceVK.hpp>
#include <Engine/Rendering/APIAbstractions/Vulkan/GeneralResourcesVK.hpp>

RenderPassVK* RenderPassVK::create(const RenderPassInfo& renderPassInfo, DeviceVK* pDevice)
{
    std::vector<VkAttachmentDescription> attachmentDescriptions;
    attachmentDescriptions.reserve(renderPassInfo.AttachmentInfos.size());
    for (const AttachmentInfo& attachmentInfo : renderPassInfo.AttachmentInfos) {
        attachmentDescriptions.push_back(convertAttachmentInfo(attachmentInfo));
    }

    std::vector<VkSubpassDescription> subpassDescriptions;
    subpassDescriptions.reserve(renderPassInfo.Subpasses.size());

    // Each subpass has a vector of color attachment references
    std::vector<std::vector<VkAttachmentReference>> attachmentReferencesColor;
    attachmentReferencesColor.resize(renderPassInfo.Subpasses.size());

    // .. and potentially a depth attachment reference
    std::vector<VkAttachmentReference> attachmentReferencesDepth(renderPassInfo.Subpasses.size());

    size_t subpassIdx = 0u;
    for (const SubpassInfo& subpassInfo : renderPassInfo.Subpasses) {
        subpassDescriptions.push_back(convertSubpassInfo(subpassInfo, attachmentReferencesColor[subpassIdx], attachmentReferencesDepth[subpassIdx]));
        subpassIdx += 1u;
    }

    std::vector<VkSubpassDependency> subpassDependencies;
    subpassDependencies.reserve(renderPassInfo.Dependencies.size());
    for (const SubpassDependency& subpassDependency : renderPassInfo.Dependencies) {
        subpassDependencies.push_back(convertSubpassDependency(subpassDependency));
    }

    VkRenderPassCreateInfo renderPassInfoVK = {};
    renderPassInfoVK.sType              = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
    renderPassInfoVK.attachmentCount    = (uint32_t)attachmentDescriptions.size();
    renderPassInfoVK.pAttachments       = attachmentDescriptions.data();
    renderPassInfoVK.subpassCount       = (uint32_t)subpassDescriptions.size();
    renderPassInfoVK.pSubpasses         = subpassDescriptions.data();
    renderPassInfoVK.dependencyCount    = (uint32_t)subpassDependencies.size();
    renderPassInfoVK.pDependencies      = subpassDependencies.data();

    VkRenderPass renderPass = VK_NULL_HANDLE;
    if (vkCreateRenderPass(pDevice->getDevice(), &renderPassInfoVK, nullptr, &renderPass) != VK_SUCCESS) {
        LOG_ERROR("Failed to create render pass");
        return nullptr;
    }

    return DBG_NEW RenderPassVK(renderPass, pDevice);
}

RenderPassVK::RenderPassVK(VkRenderPass renderPass, DeviceVK* pDevice)
    :m_RenderPass(renderPass),
    m_pDevice(pDevice)
{}

RenderPassVK::~RenderPassVK()
{
    vkDestroyRenderPass(m_pDevice->getDevice(), m_RenderPass, nullptr);
}

VkAttachmentDescription RenderPassVK::convertAttachmentInfo(const AttachmentInfo& attachmentInfo)
{
    VkAttachmentDescription attachmentDesc = {};
    attachmentDesc.format           = convertFormatToVK(attachmentInfo.Format);
    attachmentDesc.samples          = convertSampleCount(attachmentInfo.Samples);
    attachmentDesc.loadOp           = convertAttachmentLoadOp(attachmentInfo.LoadOp);
    attachmentDesc.storeOp          = convertAttachmentStoreOp(attachmentInfo.StoreOp);
    attachmentDesc.stencilLoadOp    = convertAttachmentLoadOp(attachmentInfo.StencilLoadOp);
    attachmentDesc.stencilStoreOp   = convertAttachmentStoreOp(attachmentInfo.StencilStoreOp);
    attachmentDesc.initialLayout    = convertImageLayoutFlag(attachmentInfo.InitialLayout);
    attachmentDesc.finalLayout      = convertImageLayoutFlag(attachmentInfo.FinalLayout);

    return attachmentDesc;
}

VkSubpassDescription RenderPassVK::convertSubpassInfo(const SubpassInfo& subpassInfo, std::vector<VkAttachmentReference>& attachmentReferencesColor, VkAttachmentReference& attachmentReferenceDepth)
{
    attachmentReferencesColor.reserve(subpassInfo.ColorAttachments.size());
    for (const AttachmentReference& attachmentReference : subpassInfo.ColorAttachments) {
        attachmentReferencesColor.push_back(convertAttachmentReference(attachmentReference));
    }

    if (subpassInfo.pDepthStencilAttachment) {
        attachmentReferenceDepth = convertAttachmentReference(*subpassInfo.pDepthStencilAttachment);
    }

    VkSubpassDescription subpassDesc = {};
    subpassDesc.pipelineBindPoint       = subpassInfo.PipelineBindPoint == PIPELINE_BIND_POINT::GRAPHICS ? VK_PIPELINE_BIND_POINT_GRAPHICS : VK_PIPELINE_BIND_POINT_COMPUTE;
    subpassDesc.colorAttachmentCount    = (uint32_t)attachmentReferencesColor.size();
    subpassDesc.pColorAttachments       = attachmentReferencesColor.data();
    subpassDesc.pDepthStencilAttachment = subpassInfo.pDepthStencilAttachment ? &attachmentReferenceDepth : nullptr;

    return subpassDesc;
}

VkSubpassDependency RenderPassVK::convertSubpassDependency(const SubpassDependency& subpassDependency)
{
    VkSubpassDependency subpassDependencyVK = {};
    subpassDependencyVK.srcSubpass      = subpassDependency.SrcSubpass != SUBPASS_EXTERNAL ? subpassDependency.SrcSubpass : VK_SUBPASS_EXTERNAL;
    subpassDependencyVK.dstSubpass      = subpassDependency.DstSubpass != SUBPASS_EXTERNAL ? subpassDependency.DstSubpass : VK_SUBPASS_EXTERNAL;
    subpassDependencyVK.srcStageMask    = convertPipelineStageFlags(subpassDependency.SrcStage);
    subpassDependencyVK.dstStageMask    = convertPipelineStageFlags(subpassDependency.DstStage);
    subpassDependencyVK.srcAccessMask   = convertAccessFlags(subpassDependency.SrcAccessMask);
    subpassDependencyVK.dstAccessMask   = convertAccessFlags(subpassDependency.DstAccessMask);
    subpassDependencyVK.dependencyFlags = VK_DEPENDENCY_BY_REGION_BIT;

    return subpassDependencyVK;
}

VkSampleCountFlagBits RenderPassVK::convertSampleCount(uint32_t sampleCount)
{
    switch (sampleCount) {
        case 1u:
            return VK_SAMPLE_COUNT_1_BIT;
        case 2u:
            return VK_SAMPLE_COUNT_2_BIT;
        case 4u:
            return VK_SAMPLE_COUNT_4_BIT;
        case 8u:
            return VK_SAMPLE_COUNT_8_BIT;
        case 16u:
            return VK_SAMPLE_COUNT_16_BIT;
        case 32u:
            return VK_SAMPLE_COUNT_32_BIT;
        case 64u:
            return VK_SAMPLE_COUNT_64_BIT;
        default:
            LOG_WARNING("Erroneous sample count, must be a power of 2 and <=64: %d", sampleCount);
            return VK_SAMPLE_COUNT_1_BIT;
    }
}

VkAttachmentLoadOp RenderPassVK::convertAttachmentLoadOp(ATTACHMENT_LOAD_OP loadOp)
{
    switch (loadOp) {
        case ATTACHMENT_LOAD_OP::LOAD:
            return VK_ATTACHMENT_LOAD_OP_LOAD;
        case ATTACHMENT_LOAD_OP::CLEAR:
            return VK_ATTACHMENT_LOAD_OP_CLEAR;
        case ATTACHMENT_LOAD_OP::DONT_CARE:
            return VK_ATTACHMENT_LOAD_OP_DONT_CARE;
        default:
            LOG_WARNING("Erroneous attachment load op: %d", (uint32_t)loadOp);
            return VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    }
}

VkAttachmentStoreOp RenderPassVK::convertAttachmentStoreOp(ATTACHMENT_STORE_OP storeOp)
{
    switch (storeOp) {
        case ATTACHMENT_STORE_OP::STORE:
            return VK_ATTACHMENT_STORE_OP_STORE;
        case ATTACHMENT_STORE_OP::DONT_CARE:
            return VK_ATTACHMENT_STORE_OP_DONT_CARE;
        default:
            LOG_WARNING("Erroneous attachment store op: %d", (uint32_t)storeOp);
            return VK_ATTACHMENT_STORE_OP_DONT_CARE;
    }
}

VkAttachmentReference RenderPassVK::convertAttachmentReference(const AttachmentReference& attachmentReference)
{
    VkAttachmentReference attachmentReferenceVK = {};
    attachmentReferenceVK.attachment    = attachmentReference.AttachmentIndex.has_value() ? attachmentReference.AttachmentIndex.value() : VK_ATTACHMENT_UNUSED;
    attachmentReferenceVK.layout        = convertImageLayoutFlag(attachmentReference.Layout);

    return attachmentReferenceVK;
}
