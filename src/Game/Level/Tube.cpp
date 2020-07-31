#include "Tube.hpp"

#include <Engine/ECS/ECSCore.hpp>
#include <Engine/Rendering/APIAbstractions/Device.hpp>
#include <Engine/Rendering/AssetLoaders/ModelLoader.hpp>
#include <Engine/Rendering/AssetLoaders/TextureCache.hpp>
#include <Engine/Transform.hpp>
#include <Engine/Utils/Debug.hpp>
#include <Engine/Utils/DirectXUtils.hpp>
#include <Engine/Utils/ECSUtils.hpp>
#include <Engine/Utils/Logger.hpp>

// The amount of points to add to each tube section
const unsigned addedPointsPerSection = 3;
const float maxPointDistance = 3.0f;
// Used to create a second point when a point's forward is calculated
const float deltaT = -0.0001f;
const float textureLengthReciprocal = 1/4.0f;

TubeHandler::TubeHandler(ECSCore* pECS, Device* pDevice)
    :m_pTextureCache(nullptr),
    m_pDevice(pDevice),
    m_TubeRadius(0.0f)
{
    m_pTextureCache = reinterpret_cast<TextureCache*>(pECS->getComponentSubscriber()->getComponentHandler(TID(TextureCache)));
}

Model* TubeHandler::createTube(const std::vector<DirectX::XMFLOAT3>& sectionPoints, const float radius, const unsigned faces)
{
    if (faces < 3) {
        LOG_WARNINGF("Tube must have at least 3 faces, attempted to create one with: %d", faces);
        return nullptr;
    }

    m_TubeRadius = radius;

    // Save the section points for later use
    m_TubeSections = sectionPoints;

    std::vector<TubePoint> tubePoints;
    createSections(sectionPoints, tubePoints, radius);

    std::vector<Vertex> vertices;
    vertices.resize(tubePoints.size() * faces * 2);

    const float angleBetweenFaces = DirectX::XM_2PI / faces;
    DirectX::XMVECTOR pointPosition, rotationQuat, pointUp, pointForward;
    DirectX::XMVECTOR vertexPosition, vertexNormal;

    // Incremented using the distance between each successive point
    float texCoordV = 0.0f;

    // Make a ring around every point
    for (size_t pointIdx = 0; pointIdx < tubePoints.size(); pointIdx += 1) {
        pointPosition = DirectX::XMLoadFloat3(&tubePoints[pointIdx].position);
        rotationQuat = DirectX::XMLoadFloat4(&tubePoints[pointIdx].rotationQuat);
        pointUp = DirectX::XMVector3Rotate(g_DefaultUp, rotationQuat);
        pointForward = DirectX::XMVector3Rotate(g_DefaultForward, rotationQuat);
        vertexPosition = DirectX::XMVectorAdd(pointPosition, DirectX::XMVectorScale(pointUp, radius));

        // Calculate the 'vertical' texture coordinate, which is common for each face of the ring
        TubePoint& previousPoint = tubePoints[std::max(0, (int)pointIdx - 1)];
        DirectX::XMVECTOR previousPos = DirectX::XMLoadFloat3(&previousPoint.position);
        float pointDist = DirectX::XMVectorGetX(DirectX::XMVector3Length(DirectX::XMVectorSubtract(pointPosition, previousPos)));
        texCoordV += pointDist * textureLengthReciprocal;

        // Place two vertices in each face in the ring
        for (size_t faceIdx = 0; faceIdx < faces; faceIdx += 1) {
            // Angle to rotate around the point's forward vector in order to get a vertex position
            float positionAngle = angleBetweenFaces * faceIdx;

            const float normalAngle = positionAngle + angleBetweenFaces*0.5f;
            vertexNormal = DirectX::XMVector3Rotate(pointUp, DirectX::XMQuaternionRotationAxis(pointForward, normalAngle));
            vertexNormal = DirectX::XMVectorNegate(vertexNormal);

            // Compute vertex 1
            size_t vertexIdx = pointIdx * faces * 2 + faceIdx * 2;

            DirectX::XMStoreFloat3(&vertices[vertexIdx].position, vertexPosition);
            DirectX::XMStoreFloat3(&vertices[vertexIdx].normal, vertexNormal);
            vertices[vertexIdx].txCoords = {0.0f, texCoordV};

            // Compute vertex 2
            vertexIdx += 1;
            TransformHandler::rotateAroundPoint(pointPosition, vertexPosition, pointForward, angleBetweenFaces);

            DirectX::XMStoreFloat3(&vertices[vertexIdx].position, vertexPosition);
            DirectX::XMStoreFloat3(&vertices[vertexIdx].normal, vertexNormal);
            vertices[vertexIdx].txCoords = {1.0f, texCoordV};
        }
    }

    // Make indices
    std::vector<unsigned int> indices;
    indices.resize(faces * 2 * 3 * (tubePoints.size() - 1)); // Two triangles per face with 3 vertices each

	unsigned startVertexIndex = 0;

    for (unsigned i = 0; i <= (unsigned)indices.size() - 6; i += 6) {
        // Triangle 1
        indices[i]      = startVertexIndex;
        indices[i+1]    = startVertexIndex + faces * 2;
        indices[i+2]    = startVertexIndex + 1;

        // Triangle 2
        indices[i+3] = startVertexIndex + 1;
        indices[i+4] = startVertexIndex + faces * 2;
        indices[i+5] = startVertexIndex + faces * 2 + 1;

		startVertexIndex += 2;
    }

    Model* pModel = DBG_NEW Model();
    pModel->Meshes.resize(1);

    Mesh& mesh          = pModel->Meshes.front();
    mesh.materialIndex  = 0;
    mesh.vertexCount    = vertices.size();
    mesh.indexCount     = indices.size();

    mesh.pVertexBuffer = m_pDevice->createVertexBuffer(vertices.data(), sizeof(Vertex), vertices.size());
    if (!mesh.pVertexBuffer) {
        return nullptr;
    }

    mesh.pIndexBuffer = m_pDevice->createIndexBuffer(indices.data(), indices.size());
    if (!mesh.pIndexBuffer) {
        return nullptr;
    }

    // Create material
    pModel->Materials.resize(1);
    Material& material = pModel->Materials.front();
    material.attributes.specular = {0.5f, 0.5f, 0.0f, 0.0f};

    material.textures.push_back(m_pTextureCache->loadTexture("./assets/Models/Cube.png"));

    return pModel;
}

const std::vector<DirectX::XMFLOAT3>& TubeHandler::getTubeSections() const
{
    return m_TubeSections;
}

float TubeHandler::getTubeRadius() const
{
    return m_TubeRadius;
}

void TubeHandler::createSections(const std::vector<DirectX::XMFLOAT3>& sectionPoints, std::vector<TubePoint>& tubePoints, const float radius)
{
    // Rough estimate of the amount of points in the tube
    tubePoints.reserve((sectionPoints.size()-1) * addedPointsPerSection + sectionPoints.size());

    float tStepPerPoint = 1.0f/(addedPointsPerSection + 1);

    // Every two successive points make up a tube section
    for (size_t section = 0; section < sectionPoints.size() - 1; section += 1) {
        createSectionPoint(sectionPoints, tubePoints, section, 0.0f);

        for (size_t i = 0; i < addedPointsPerSection; i += 1) {
            float T = tStepPerPoint * (i+1);

            createSectionPoint(sectionPoints, tubePoints, section, T);
        }
    }

    // .. and the last point
    createSectionPoint(sectionPoints, tubePoints, sectionPoints.size()-1, 1.0f);
}

void TubeHandler::createSectionPoint(const std::vector<DirectX::XMFLOAT3>& sectionPoints, std::vector<TubePoint>& tubePoints, size_t sectionIdx, float T)
{
    // Get four control points for catmull-rom
    size_t idx0, idx1, idx2, idx3;
    idx0 = std::max((int)sectionIdx-1, 0);
    idx1 = sectionIdx;
    idx2 = std::min(sectionIdx+1, sectionPoints.size()-1);
    idx3 = std::min(sectionIdx+2, sectionPoints.size()-1);

    DirectX::XMVECTOR P[4];
    P[0] = DirectX::XMLoadFloat3(&sectionPoints[idx0]);
    P[1] = DirectX::XMLoadFloat3(&sectionPoints[idx1]);
    P[2] = DirectX::XMLoadFloat3(&sectionPoints[idx2]);
    P[3] = DirectX::XMLoadFloat3(&sectionPoints[idx3]);

    TubePoint newPoint;
    DirectX::XMStoreFloat3(&newPoint.position, DirectX::XMVectorCatmullRom(P[0], P[1], P[2], P[3], T));
    DirectX::XMStoreFloat4(&newPoint.rotationQuat, DirectX::XMQuaternionIdentity());
    TransformHandler::setForward(newPoint.rotationQuat, DirectX::XMVector3Normalize(catmullRomDerivative(P[0], P[1], P[2], P[3], T)));

    tubePoints.push_back(newPoint);
}
