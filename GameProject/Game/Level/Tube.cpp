#include "Tube.hpp"

#include <Engine/Rendering/AssetLoaders/ModelLoader.hpp>
#include <Engine/Rendering/AssetLoaders/TextureLoader.hpp>
#include <Engine/ECS/SystemSubscriber.hpp>
#include <Engine/Rendering/Display.hpp>
#include <Engine/Transform.hpp>
#include <Engine/Utils/DirectXUtils.hpp>
#include <Engine/Utils/Logger.hpp>

// The amount of points to add to each tube section
const unsigned addedPointsPerSection = 3;
const float maxPointDistance = 3.0f;
// Used to create a second point when a point's forward is calculated
const float deltaT = -0.0001f;
const float textureLengthReciprocal = 1/4.0f;

TubeHandler::TubeHandler(SystemSubscriber* sysSubscriber, ID3D11Device* device)
    :ComponentHandler({}, sysSubscriber, std::type_index(typeid(TubeHandler))),
    device(device)
{
    std::type_index tid_textureLoader = std::type_index(typeid(TextureLoader));

    this->textureLoader = static_cast<TextureLoader*>(sysSubscriber->getComponentHandler(tid_textureLoader));

    std::vector<ComponentRegistration> compRegs = {};
    this->registerHandler(&compRegs);
}

TubeHandler::~TubeHandler()
{
    for (size_t i = 0; i < tubes.size(); i += 1) {
        releaseModel(&tubes[i]);
    }
}

Model* TubeHandler::createTube(const std::vector<DirectX::XMFLOAT3>& sectionPoints, const float radius, const unsigned faces)
{
    if (faces < 3) {
        Logger::LOG_WARNING("Tube must have at least 3 faces, attempted to create one with: %d", faces);
        return nullptr;
    }

    // Save the section points for later use
    this->tubeSections = sectionPoints;

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
        pointUp = DirectX::XMVector3Rotate(defaultUp, rotationQuat);
        pointForward = DirectX::XMVector3Rotate(defaultForward, rotationQuat);
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
        indices[i] = startVertexIndex;
        indices[i+1] = startVertexIndex + faces*2;
        indices[i+2] = startVertexIndex + 1;

        // Triangle 2
        indices[i+3] = startVertexIndex + 1;
        indices[i+4] = startVertexIndex + faces*2;
        indices[i+5] = startVertexIndex + faces*2 + 1;

		startVertexIndex += 2;
    }

    tubes.push_back(Model());
    Model& model = tubes.front();
    model.meshes.resize(1);
    Mesh& mesh = model.meshes.front();
    mesh.materialIndex = 0;
    mesh.vertexCount = vertices.size();
    mesh.indexCount = indices.size();

    // Create vertex buffer
    D3D11_BUFFER_DESC bufferDesc;
    ZeroMemory(&bufferDesc, sizeof(D3D11_BUFFER_DESC));
    bufferDesc.ByteWidth = (UINT)(sizeof(Vertex) * vertices.size());
    bufferDesc.Usage = D3D11_USAGE_IMMUTABLE;
    bufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
    bufferDesc.CPUAccessFlags = 0;
    bufferDesc.MiscFlags = 0;
    bufferDesc.StructureByteStride = 0;

    D3D11_SUBRESOURCE_DATA bufferData;
    bufferData.pSysMem = &vertices.front();
    bufferData.SysMemPitch = 0;
    bufferData.SysMemSlicePitch = 0;

    HRESULT hr = device->CreateBuffer(&bufferDesc, &bufferData, &mesh.vertexBuffer);
    if (FAILED(hr)) {
        Logger::LOG_WARNING("Failed to create vertex buffer: %s", hresultToString(hr).c_str());
        return nullptr;
    }

    // Create index buffer
    bufferDesc.ByteWidth = (UINT)(sizeof(unsigned int) * indices.size());
    bufferDesc.Usage = D3D11_USAGE_IMMUTABLE;
    bufferDesc.BindFlags = D3D11_BIND_INDEX_BUFFER;

    bufferData.pSysMem = &indices.front();
    bufferData.SysMemPitch = 0;
    bufferData.SysMemSlicePitch = 0;

    hr = device->CreateBuffer(&bufferDesc, &bufferData, &mesh.indexBuffer);
    if (FAILED(hr)) {
        Logger::LOG_WARNING("Failed to create index buffer: %s", hresultToString(hr).c_str());
        return nullptr;
    }

    // Create material
    model.materials.resize(1);
    Material& material = model.materials.front();
    material.attributes.specular = {0.5f, 0.5f, 0.0f, 0.0f};

    material.textures.push_back(textureLoader->loadTexture("./Game/Assets/Models/Cube.png", TX_TYPE::DIFFUSE));

    return &model;
}

const std::vector<DirectX::XMFLOAT3>& TubeHandler::getTubeSections() const
{
    return tubeSections;
}

void TubeHandler::createSections(const std::vector<DirectX::XMFLOAT3>& sectionPoints, std::vector<TubePoint>& tubePoints, const float radius)
{
    // Rough estimate of the amount of points in the tube
    tubePoints.reserve((sectionPoints.size()-1) * addedPointsPerSection + sectionPoints.size());

    float tStepPerPoint = 1.0f/(addedPointsPerSection + 1);

    // Every two successive points make up a tube section
    for (size_t section = 0; section < sectionPoints.size() - 1; section += 1) {
        createTubePoint(sectionPoints, tubePoints, section, 0.0f);

        for (size_t i = 0; i < addedPointsPerSection; i += 1) {
            float T = tStepPerPoint * (i+1);

            createTubePoint(sectionPoints, tubePoints, section, T);
        }
    }

    // .. and the last point
    createTubePoint(sectionPoints, tubePoints, sectionPoints.size()-1, 1.0f);
}

DirectX::XMFLOAT3 TubeHandler::getPointForward(DirectX::XMVECTOR P[], DirectX::XMVECTOR& pointPos, float T0, float T1)
{
    DirectX::XMVECTOR temp = DirectX::XMVectorCatmullRom(P[0], P[1], P[2], P[3], T1-T0);
    temp = DirectX::XMVector3Normalize(DirectX::XMVectorSubtract(pointPos, temp));

    DirectX::XMFLOAT3 forward;
    DirectX::XMStoreFloat3(&forward, temp);

    return forward;
}

void TubeHandler::createTubePoint(const std::vector<DirectX::XMFLOAT3>& sectionPoints, std::vector<TubePoint>& tubePoints, size_t sectionIdx, float T)
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

    DirectX::XMVECTOR newPointPos = DirectX::XMVectorCatmullRom(P[0], P[1], P[2], P[3], T);

    // Invert deltaT if it's the first point in the tube
    float dT = deltaT + 2.0f * deltaT * (sectionIdx == 0);
    DirectX::XMFLOAT3 pointForward = getPointForward(P, newPointPos, T, T + dT);

    TubePoint newPoint;
    DirectX::XMStoreFloat3(&newPoint.position, newPointPos);
    newPoint.rotationQuat = TransformHandler::getRotationQuaternion(pointForward);

    tubePoints.push_back(newPoint);
}