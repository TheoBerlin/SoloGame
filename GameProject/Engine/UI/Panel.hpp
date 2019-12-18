#pragma once

#define NOMINMAX

#include <Engine/ECS/ComponentHandler.hpp>
#include <Engine/Utils/IDVector.hpp>
#include <DirectXMath.h>
#include <d3d11.h>

struct UIPanel
{
    // Position and size are specified in factors [0, 1]. The position describes the bottom left corner of the panel.
    // Note that the final size of the panel scales with the aspect ratio of the window.
    DirectX::XMFLOAT2 position, size;
    DirectX::XMFLOAT4 color;

    ID3D11ShaderResourceView* texture;
};

const std::type_index tid_UIPanel = std::type_index(typeid(UIPanel));

struct UIButton
{
    DirectX::XMFLOAT4 defaultColor, hoverColor, pressColor;
    // For now, buttons are handled after systems have been updated.
    // Perhaps later, this could be changed by specifying what component types the function affects, and with what permissions.
    std::function<void()> onPress;
};

const std::type_index tid_UIButton = std::type_index(typeid(UIButton));

class TextureLoader;

class UIHandler : public ComponentHandler
{
public:
    UIHandler(SystemSubscriber* sysSubscriber);
    ~UIHandler();

    void createPanel(Entity entity, DirectX::XMFLOAT2 pos, DirectX::XMFLOAT2 size, DirectX::XMFLOAT4 color,
    std::string texturePath = "./Game/Assets/Models/Solid_White.png");

    void createPanel(Entity entity, DirectX::XMFLOAT2 pos, DirectX::XMFLOAT2 size, DirectX::XMFLOAT4 color, ID3D11ShaderResourceView* texture);

    // Requires that the entity has a UI panel already
    void createButton(Entity entity, DirectX::XMFLOAT4 hoverColor, DirectX::XMFLOAT4 pressColor,
    std::function<void()> onPress);

    IDVector<UIPanel> panels;
    IDVector<UIButton> buttons;

private:
    TextureLoader* textureLoader;
};
