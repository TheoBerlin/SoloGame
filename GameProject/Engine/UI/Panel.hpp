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

class TextureLoader;

class UIHandler : public ComponentHandler
{
public:
    UIHandler(SystemSubscriber* sysSubscriber);
    ~UIHandler();

    void createPanel(Entity entity, DirectX::XMFLOAT2 pos, DirectX::XMFLOAT2 size, DirectX::XMFLOAT4 color,
    std::string texturePath = "./Game/Assets/Models/Solid_White.png");

    IDVector<UIPanel> panels;

private:
    TextureLoader* textureLoader;
};
