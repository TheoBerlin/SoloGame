#pragma once

#include <Engine/ECS/Component.hpp>
#include <Engine/ECS/ComponentOwner.hpp>
#include <Engine/Rendering/APIAbstractions/Texture.hpp>
#include <Engine/Utils/ECSUtils.hpp>
#include <Engine/Utils/IDVector.hpp>

#include <DirectXMath.h>
#include <string>

enum TX_HORIZONTAL_ALIGNMENT {
    TX_HORIZONTAL_ALIGNMENT_LEFT,
    TX_HORIZONTAL_ALIGNMENT_CENTER,
    TX_HORIZONTAL_ALIGNMENT_RIGHT,
    TX_HORIZONTAL_ALIGNMENT_EXPLICIT  // The horizontal position is specified explicitly in the attachment info
};

enum TX_VERTICAL_ALIGNMENT {
    TX_VERTICAL_ALIGNMENT_TOP,
    TX_VERTICAL_ALIGNMENT_CENTER,
    TX_VERTICAL_ALIGNMENT_BOTTOM,
    TX_VERTICAL_ALIGNMENT_EXPLICIT  // The vertical position is specified explicitly in the attachment info
};

enum TX_SIZE {
    TX_SIZE_STRETCH,
    TX_SIZE_CLIENT_RESOLUTION_DEPENDENT,    // The size is dependent on the texture's size relative to the size of the client
    TX_SIZE_EXPLICIT                        // The size relative to the panel is specified explicitly in the attachment info
};

struct TextureAttachmentInfo {
    TX_HORIZONTAL_ALIGNMENT horizontalAlignment;
    TX_VERTICAL_ALIGNMENT verticalAlignment;
    TX_SIZE sizeSetting;
    DirectX::XMFLOAT2 explicitPosition;
    DirectX::XMFLOAT2 explicitSize;
};

struct TextureAttachment {
    // Position and size are specified as [0, 1], and are relative to the panel's position and size.
    DirectX::XMFLOAT2 position, size;
    std::shared_ptr<Texture> texture;
};

struct UIPanelComponent {
    DECL_COMPONENT(UIPanelComponent);
    // Position and size are specified in factors [0, 1]. The position describes the bottom left corner of the panel.
    // Note that the final size of the panel scales with the aspect ratio of the window.
    DirectX::XMFLOAT2 position, size;
    /*  Highlights are applied with the equation: finalColor = txColor + highlightFactor * (highlight * txColor)
        A negative highlight factor will make the panel darker */
    DirectX::XMFLOAT4 highlight;
    float highlightFactor;
    std::vector<TextureAttachment> textures;
    Texture* texture;
};

struct UIButtonComponent {
    DECL_COMPONENT(UIButtonComponent);
    DirectX::XMFLOAT4 defaultHighlight, hoverHighlight, pressHighlight;
    // For now, buttons are handled after systems have been updated.
    // Perhaps later, this could be changed by specifying what component types the function affects, and with what permissions.
    std::function<void()> onPress;
};

class Device;

struct AttachmentRenderResources {
    DescriptorSet* pDescriptorSet;
    IBuffer* pAttachmentBuffer;
};

class UIHandler : ComponentOwner
{
public:
    UIHandler(Device* pDevice);
    ~UIHandler();

    bool Init();

    UIPanelComponent CreatePanel(DirectX::XMFLOAT2 pos, DirectX::XMFLOAT2 size, const DirectX::XMFLOAT4& highlight, float highlightFactor);
    void AttachTextures(Entity entity, const TextureAttachmentInfo* pAttachmentInfos, const std::shared_ptr<Texture>* pTextureReferences, size_t textureCount);

private:
    void PanelDestructor(UIPanelComponent& panel, Entity entity);

    bool CreateRenderPass();
    bool CreateDescriptorSetLayout();
    bool CreatePipeline();

    // Creates a texture for a panel, which can be used as both a RTV and SRV
    void CreatePanelTexture(UIPanelComponent& panel);
    void CreateTextureAttachment(TextureAttachment& attachment, const TextureAttachmentInfo& attachmentInfo, const std::shared_ptr<Texture>& texture, const UIPanelComponent& panel);
    void RenderTexturesOntoPanel(std::vector<TextureAttachment>& attachments, UIPanelComponent& panel);
    bool CreatePanelRenderResources(std::vector<AttachmentRenderResources>& renderResources, std::vector<TextureAttachment>& attachments);
    Framebuffer* CreateFramebuffer(UIPanelComponent& panel);

private:
    Device* m_pDevice;
    ICommandPool* m_pCommandPool;
    ICommandList* m_pCommandList;

    IBuffer* m_pQuadVertices;
    ISampler* m_pAniSampler;

    IDescriptorSetLayout* m_pDescriptorSetLayout;

    IRenderPass* m_pRenderPass;
    IPipelineLayout* m_pPipelineLayout;
    IPipeline* m_pPipeline;
};
