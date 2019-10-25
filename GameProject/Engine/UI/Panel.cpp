#include "Panel.hpp"

#include <Engine/ECS/SystemSubscriber.hpp>
#include <Engine/Rendering/AssetLoaders/TextureLoader.hpp>

UIHandler::UIHandler(SystemSubscriber* sysSubscriber)
    :ComponentHandler({tid_UIPanel}, sysSubscriber, std::type_index(typeid(UIHandler)))
{
    std::vector<ComponentRegistration> compRegs = {
        {tid_UIPanel, &panels}
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
