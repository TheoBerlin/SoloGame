#pragma once

#include <DirectXMath.h>

#include <vector>

struct ModelComponent;
class Device;
class TextureCache;

struct TubePoint {
    DirectX::XMFLOAT3 position;
    DirectX::XMFLOAT4 rotationQuat;
};

class TubeHandler
{
public:
    TubeHandler();
    ~TubeHandler() = default;

    ModelComponent CreateTube(const std::vector<DirectX::XMFLOAT3>& sectionPoints, float radius, unsigned faces);

    const std::vector<DirectX::XMFLOAT3>& GetTubeSections() const { return m_TubeSections; }
    float GetTubeRadius() const { return m_TubeRadius; }

private:
    // Populate tube sections with enough points to create smooth curves and to avoid stretching textures. The resulting points are stored in tubePoints.
    static void CreateSections(const std::vector<DirectX::XMFLOAT3>& sectionPoints, std::vector<TubePoint>& tubePoints, float radius);

    // Creates a point in the tube. sectionPoints contains the sparse control points, and tubePoints contains the densely packed points.
    static void CreateSectionPoint(const std::vector<DirectX::XMFLOAT3>& sectionPoints, std::vector<TubePoint>& tubePoints, size_t sectionIdx, float T);

private:
    std::vector<DirectX::XMFLOAT3> m_TubeSections;
    float m_TubeRadius;
};
