#pragma once

#define NOMINMAX
#include <d3d11.h>
#include <DirectXMath.h>
#include <vector>
#include <Engine/ECS/ComponentHandler.hpp>

struct Model;
class ModelLoader;
class SystemSubscriber;
class TextureLoader;

struct TubePoint {
    DirectX::XMFLOAT3 position;
    DirectX::XMFLOAT4 rotationQuat;
};

class TubeHandler
{
public:
    TubeHandler(SystemSubscriber* sysSubscriber, ID3D11Device* device);
    ~TubeHandler();

    Model* createTube(const std::vector<TubePoint>& points, const float radius, const unsigned faces);

private:
    TextureLoader* textureLoader;
    ID3D11Device* device;

    std::vector<Model> tubes;

    // Creates more points in between the listed points to create smooth curves and to avoid stretching textures
    void createPoints();
};
