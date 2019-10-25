#include "Panel.hpp"

UIHandler::UIHandler(SystemSubscriber* sysSubscriber)
    :ComponentHandler({tid_UIPanel}, sysSubscriber, std::type_index(typeid(UIHandler)))
{
    std::vector<ComponentRegistration> compRegs = {
        {tid_UIPanel, &panels}
    };

    this->registerHandler(&compRegs);
}

UIHandler::~UIHandler()
{}

void UIHandler::createPanel(Entity entity, DirectX::XMFLOAT2 pos, DirectX::XMFLOAT2 size, DirectX::XMFLOAT4 color)
{
    UIPanel panel;
    panel.position = pos;
    panel.size = size;
    panel.color = color;

    panels.push_back(panel, entity);
    this->registerComponent(tid_UIPanel, entity);
}
