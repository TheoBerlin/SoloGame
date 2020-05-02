#pragma once

#define NOMINMAX
#include <DirectXMath.h>
#include <vector>
#include <Engine/ECS/ComponentHandler.hpp>

struct Model;
class DeviceDX11;
class ModelLoader;
class ComponentSubscriber;
class TextureLoader;

struct TubePoint {
    DirectX::XMFLOAT3 position;
    DirectX::XMFLOAT4 rotationQuat;
};

class TubeHandler : public ComponentHandler
{
public:
    TubeHandler(ECSCore* pECS, DeviceDX11* pDevice);
    ~TubeHandler();

    virtual bool initHandler() override;

    Model* createTube(const std::vector<DirectX::XMFLOAT3>& sectionPoints, const float radius, const unsigned faces);

    const std::vector<DirectX::XMFLOAT3>& getTubeSections() const;
    float getTubeRadius() const;

private:
    // Populate tube sections with enough points to create smooth curves and to avoid stretching textures. The resulting points are stored in tubePoints.
    void createSections(const std::vector<DirectX::XMFLOAT3>& sectionPoints, std::vector<TubePoint>& tubePoints, const float radius);

    // Creates a point in the tube. sectionPoints contains the sparse control points, and tubePoints contains the densely packed points.
    void createTubePoint(const std::vector<DirectX::XMFLOAT3>& sectionPoints, std::vector<TubePoint>& tubePoints, size_t pointIdx, float T);

private:
    TextureLoader* m_pTextureLoader;
    DeviceDX11* m_pDevice;

    std::vector<DirectX::XMFLOAT3> m_TubeSections;
    std::vector<Model> m_Tubes;
    float m_TubeRadius;

    // Gets the forward vector of a point at T0 pointing at T1 in a catmull-rom curve
    DirectX::XMFLOAT3 getPointForward(DirectX::XMVECTOR P[], DirectX::XMVECTOR& pointPos, float T0, float T1);
};
