#include "Panel.hpp"

#include <Engine/ECS/SystemSubscriber.hpp>
#include <Engine/Rendering/AssetLoaders/TextureLoader.hpp>
#include <Engine/Utils/Logger.hpp>

UIHandler::UIHandler(SystemSubscriber* sysSubscriber, unsigned int clientWidth, unsigned int clientHeight)
    :ComponentHandler({tid_UIPanel}, sysSubscriber, std::type_index(typeid(UIHandler))),
    m_ClientWidth(clientWidth),
    m_ClientHeight(clientHeight)
{
    std::vector<ComponentRegistration> compRegs = {
        {tid_UIPanel, &panels},
        {tid_UIButton, &buttons}
    };

    this->registerHandler(&compRegs);

    std::type_index tid_textureLoader = std::type_index(typeid(TextureLoader));
    textureLoader = static_cast<TextureLoader*>(sysSubscriber->getComponentHandler(tid_textureLoader));
}

UIHandler::~UIHandler()
{}

void UIHandler::createPanel(Entity entity, DirectX::XMFLOAT2 pos, DirectX::XMFLOAT2 size, DirectX::XMFLOAT4 highlight, float highlightFactor)
{
    UIPanel panel;
    panel.position = pos;
    panel.size = size;
    panel.highlightFactor = highlightFactor;
    panel.highlight = highlight;

    panels.push_back(panel, entity);
    this->registerComponent(tid_UIPanel, entity);
}

void UIHandler::attachTexture(Entity entity, const TextureAttachmentInfo& attachmentInfo, ID3D11ShaderResourceView* texture)
{
    if (!panels.hasElement(entity)) {
        Logger::LOG_WARNING("Tried to attach a texture to a non-existing UI panel, entity: %d", entity);
        return;
    }

    const UIPanel& panel = panels.indexID(entity);

    TextureAttachment attachment = {};
    createTextureAttachment(attachment, attachmentInfo, texture, panel);

    panels.indexID(entity).textures.push_back(attachment);
}

void UIHandler::attachTexture(Entity entity, const TextureAttachmentInfo& attachmentInfo, std::string texturePath)
{
    ID3D11ShaderResourceView* texture = textureLoader->loadTexture(texturePath).srv;
    attachTexture(entity, attachmentInfo, texture);
}

void UIHandler::createButton(Entity entity, DirectX::XMFLOAT4 hoverHighlight, DirectX::XMFLOAT4 pressHighlight,
    std::function<void()> onPress)
{
    if (!panels.hasElement(entity)) {
        Logger::LOG_WARNING("Tried to create a UI button for entity (%d) which does not have a UI panel", entity);
        return;
    }

    DirectX::XMFLOAT4 defaultHighlight = panels.indexID(entity).highlight;

    buttons.push_back({
        defaultHighlight,
        hoverHighlight,
        pressHighlight,
        onPress
    }, entity);

    this->registerComponent(tid_UIButton, entity);
}

void UIHandler::createTextureAttachment(TextureAttachment& attachment, const TextureAttachmentInfo& attachmentInfo, ID3D11ShaderResourceView* texture, const UIPanel& panel)
{
    attachment.texture = texture;

    // Set size
    if (attachmentInfo.sizeSetting == TX_SIZE_STRETCH) {
        attachment.size = {1.0f, 1.0f};
        attachment.position = {0.0f, 0.0f};
        return;
    } else if (attachmentInfo.sizeSetting == TX_SIZE_CLIENT_RESOLUTION_DEPENDENT) {
        // Get the resolution of the texture
        ID3D11Resource* resource;
        texture->GetResource(&resource);

        ID3D11Texture2D* tx2D = static_cast<ID3D11Texture2D*>(resource);
        D3D11_TEXTURE2D_DESC txDesc = {};
        tx2D->GetDesc(&txDesc);

        const DirectX::XMFLOAT2& panelSize = panel.size;
        attachment.size = {panelSize.x * (float)txDesc.Width / m_ClientWidth, panelSize.y * (float)txDesc.Height / m_ClientHeight};
    }

    // Set position
    switch (attachmentInfo.horizontalAlignment) {
        case TX_HORIZONTAL_ALIGNMENT_LEFT:
            attachment.position.x = 0.0f;
            break;
        case TX_HORIZONTAL_ALIGNMENT_CENTER:
            attachment.position.x = 0.5f;
            break;
        case TX_HORIZONTAL_ALIGNMENT_RIGHT:
            attachment.position.x = 1.0f;
            break;
        case TX_HORIZONTAL_ALIGNMENT_EXPLICIT:
            attachment.position.x = attachmentInfo.explicitPosition.x;
            break;
    }

    switch (attachmentInfo.verticalAlignment) {
        case TX_VERTICAL_ALIGNMENT_TOP:
            attachment.position.y = 1.0f;
            break;
        case TX_VERTICAL_ALIGNMENT_CENTER:
            attachment.position.y = 0.5f;
            break;
        case TX_VERTICAL_ALIGNMENT_BOTTOM:
            attachment.position.y = 0.0f;
            break;
        case TX_VERTICAL_ALIGNMENT_EXPLICIT:
            attachment.position.y = attachmentInfo.explicitPosition.y;
            break;
    }
}
