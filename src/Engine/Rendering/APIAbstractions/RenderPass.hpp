#pragma once

#include <Engine/Rendering/APIAbstractions/GeneralResources.hpp>
#include <Engine/Rendering/APIAbstractions/Texture.hpp>

#include <glm/glm.hpp>

#include <optional>
#include <vector>

enum class ATTACHMENT_LOAD_OP : uint32_t {
    LOAD,
    CLEAR,
    DONT_CARE
};

enum class ATTACHMENT_STORE_OP : uint32_t {
    STORE,
    DONT_CARE
};

struct AttachmentInfo {
    RESOURCE_FORMAT Format;
    uint32_t Samples;
    ATTACHMENT_LOAD_OP LoadOp;
    ATTACHMENT_STORE_OP StoreOp;
    ATTACHMENT_LOAD_OP StencilLoadOp;
    ATTACHMENT_STORE_OP StencilStoreOp;
    TEXTURE_LAYOUT InitialLayout;
    TEXTURE_LAYOUT FinalLayout;
};

enum class PIPELINE_BIND_POINT {
    GRAPHICS,
    COMPUTE
};

struct AttachmentReference {
    std::optional<uint32_t> AttachmentIndex; // No value in case the attachment is not used
    TEXTURE_LAYOUT Layout;
};

struct SubpassInfo {
    PIPELINE_BIND_POINT PipelineBindPoint;
    std::vector<AttachmentReference> ColorAttachments;
    AttachmentReference* pDepthStencilAttachment;
};

#define SUBPASS_EXTERNAL UINT32_MAX

struct SubpassDependency {
    uint32_t SrcSubpass; // Subpass index or SUBPASS_EXTERNAL
    uint32_t DstSubpass; // Subpass index or SUBPASS_EXTERNAL
    PIPELINE_STAGE SrcStage;
    PIPELINE_STAGE DstStage;
    RESOURCE_ACCESS SrcAccessMask;
    RESOURCE_ACCESS DstAccessMask;
    DEPENDENCY_FLAG DependencyFlags;
};

struct RenderPassInfo {
    std::vector<AttachmentInfo> AttachmentInfos;
    std::vector<SubpassInfo> Subpasses;
    std::vector<SubpassDependency> Dependencies;
};

class Framebuffer;

struct ClearDepthStencilValue {
    float Depth;
    uint32_t Stencil;
};

union ClearColorValue {
    float       float32[4];
    int32_t     int32[4];
    uint32_t    uint32[4];
};

union ClearValue {
    ClearColorValue ClearColorValue;
    ClearDepthStencilValue DepthStencilValue;
};

struct RenderPassBeginInfo {
    Framebuffer* pFramebuffer;
    ClearValue* pClearValues;
    uint32_t ClearValueCount;
    COMMAND_LIST_LEVEL RecordingListType; // The type of command list that will record the first subpass' commands
};

class IRenderPass
{
public:
    IRenderPass() = default;
    virtual ~IRenderPass() {};
};
