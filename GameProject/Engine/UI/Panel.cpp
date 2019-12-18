#include "Panel.hpp"

#include <Engine/ECS/SystemSubscriber.hpp>
#include <Engine/Rendering/AssetLoaders/TextureLoader.hpp>
#include <Engine/Utils/Logger.hpp>

UIHandler::UIHandler(SystemSubscriber* sysSubscriber)
    :ComponentHandler({tid_UIPanel}, sysSubscriber, std::type_index(typeid(UIHandler)))
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

void UIHandler::createPanel(Entity entity, DirectX::XMFLOAT2 pos, DirectX::XMFLOAT2 size, DirectX::XMFLOAT4 color, std::string texturePath)
{
    UIPanel panel;
    panel.position = pos;
    panel.size = size;
    panel.color = color;
    panel.texture = textureLoader->loadTexture(texturePath).srv;

    panels.push_back(panel, entity);
    this->registerComponent(tid_UIPanel, entity);
}

void UIHandler::createPanel(Entity entity, DirectX::XMFLOAT2 pos, DirectX::XMFLOAT2 size, DirectX::XMFLOAT4 color, ID3D11ShaderResourceView* texture)
{
    UIPanel panel;
    panel.position = pos;
    panel.size = size;
    panel.color = color;
    panel.texture = texture;

    panels.push_back(panel, entity);
    this->registerComponent(tid_UIPanel, entity);
}

void UIHandler::createButton(Entity entity, DirectX::XMFLOAT4 hoverColor, DirectX::XMFLOAT4 pressColor,
    std::function<void()> onPress)
{
    if (!panels.hasElement(entity)) {
        Logger::LOG_WARNING("Tried to create a UI button for entity (%d) which does not have a UI panel", entity);
        return;
    }

    DirectX::XMFLOAT4 defaultColor = panels.indexID(entity).color;

    buttons.push_back({
        defaultColor,
        hoverColor,
        pressColor,
        onPress
    }, entity);

    this->registerComponent(tid_UIButton, entity);
}
