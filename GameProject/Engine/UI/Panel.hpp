#pragma once

#define NOMINMAX

#include <Engine/ECS/ComponentHandler.hpp>
#include <Engine/Rendering/APIAbstractions/Texture.hpp>
#include <Engine/Utils/ECSUtils.hpp>
#include <Engine/Utils/IDVector.hpp>

#include <DirectXMath.h>
#include <d3d11.h>
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

struct UIPanel {
    // Position and size are specified in factors [0, 1]. The position describes the bottom left corner of the panel.
    // Note that the final size of the panel scales with the aspect ratio of the window.
    DirectX::XMFLOAT2 position, size;
    /*  Highlights are applied with the equation: finalColor = txColor + highlightFactor * (highlight * txColor)
        A negative highlight factor will make the panel darker */
    float highlightFactor;
    DirectX::XMFLOAT4 highlight;
    std::vector<TextureAttachment> textures;
    Texture* texture;
};

const std::type_index tid_UIPanel = TID(UIPanel);

struct UIButton {
    DirectX::XMFLOAT4 defaultHighlight, hoverHighlight, pressHighlight;
    // For now, buttons are handled after systems have been updated.
    // Perhaps later, this could be changed by specifying what component types the function affects, and with what permissions.
    std::function<void()> onPress;
};

const std::type_index tid_UIButton = TID(UIButton);

class BlendState;
class BufferDX11;
class Display;
class IBuffer;
class ICommandList;
class IRasterizerState;
class ISampler;
class Device;
class Window;
struct Program;

class UIHandler : public ComponentHandler
{
public:
    UIHandler(ECSCore* pECS, Device* pDevice, Window* pWindow);
    ~UIHandler();

    virtual bool initHandler() override;

    void createPanel(Entity entity, DirectX::XMFLOAT2 pos, DirectX::XMFLOAT2 size, DirectX::XMFLOAT4 highlight, float highlightFactor);
    void attachTextures(Entity entity, const TextureAttachmentInfo* pAttachmentInfos, std::shared_ptr<Texture>* pTextureReferences, size_t textureCount);

    // Requires that the entity has a UI panel already
    void createButton(Entity entity, DirectX::XMFLOAT4 hoverHighlight, DirectX::XMFLOAT4 pressHighlight, std::function<void()> onPress);

    IDDVector<UIPanel> panels;
    IDDVector<UIButton> buttons;

private:
    // Creates a texture for a panel, which can be used as both a RTV and SRV
    void createPanelTexture(UIPanel& panel);
    void createTextureAttachment(TextureAttachment& attachment, const TextureAttachmentInfo& attachmentInfo, std::shared_ptr<Texture>& texture, const UIPanel& panel);
    void renderTexturesOntoPanel(std::vector<TextureAttachment>& attachments, UIPanel& panel);

private:
    Device* m_pDevice;
    ICommandList* m_pCommandList;

    Program* m_pUIProgram;
    IBuffer* m_pPerObjectBuffer;
    BufferDX11* m_pQuadVertices;
    ISampler* m_pAniSampler;
    BlendState* m_pBlendState;
    IRasterizerState* m_pRasterizerState;

    unsigned int m_ClientWidth, m_ClientHeight;
};
