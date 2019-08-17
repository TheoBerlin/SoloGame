#include "Tube.hpp"

#include <Engine/Rendering/AssetLoaders/ModelLoader.hpp>
#include <Engine/Rendering/AssetLoaders/TextureLoader.hpp>
#include <Engine/ECS/SystemSubscriber.hpp>
#include <Engine/Rendering/Display.hpp>
#include <Engine/Transform.hpp>
#include <Engine/Utils/DirectXUtils.hpp>
#include <Engine/Utils/Logger.hpp>

TubeHandler::TubeHandler(SystemSubscriber* sysSubscriber, ID3D11Device* device)
    :device(device)
{
    std::type_index tid_textureLoader = std::type_index(typeid(TextureLoader));

    this->textureLoader = static_cast<TextureLoader*>(sysSubscriber->getComponentHandler(tid_textureLoader));
}

TubeHandler::~TubeHandler()
{
    for (size_t i = 0; i < tubes.size(); i += 1) {
        releaseModel(&tubes[i]);
    }
}

Model* TubeHandler::createTube(const std::vector<TubePoint>& points, const float radius, const unsigned faces)
{
    if (faces < 3) {
        Logger::LOG_WARNING("Tube must have at least 3 faces, attempted to create one with: %d", faces);
        return nullptr;
    }

    if (points.size() % 2 != 0) {
        Logger::LOG_WARNING("Tube must have an even amount of points to be properly textured, attempted to create one with: %d", points.size());
        return nullptr;
    }

    std::vector<Vertex> vertices;
    vertices.resize(points.size() * faces * 2);

    const float angleBetweenFaces = DirectX::XM_2PI / faces;
    DirectX::XMVECTOR pointPosition, rotationQuat, pointUp, pointForward;
    DirectX::XMVECTOR vertexPosition, vertexNormal;

    // Make a ring around every point
    for (size_t pointIdx = 0; pointIdx < points.size(); pointIdx += 1) {
        pointPosition = DirectX::XMLoadFloat3(&points[pointIdx].position);
        rotationQuat = DirectX::XMLoadFloat4(&points[pointIdx].rotationQuat);
        pointUp = DirectX::XMVector3Rotate(defaultUp, rotationQuat);
        pointForward = DirectX::XMVector3Rotate(defaultForward, rotationQuat);
        vertexPosition = DirectX::XMVectorAdd(pointPosition, DirectX::XMVectorScale(pointUp, radius));

        // Place two vertices in each face in the ring
        for (size_t faceIdx = 0; faceIdx < faces; faceIdx += 1) {
            // Angle to rotate around the point's forward vector in order to get a vertex position
            float positionAngle = angleBetweenFaces * faceIdx;

            const float normalAngle = positionAngle + angleBetweenFaces*0.5f;
            vertexNormal = DirectX::XMVector3Rotate(pointUp, DirectX::XMQuaternionRotationAxis(pointForward, normalAngle));
            vertexNormal = DirectX::XMVectorNegate(vertexNormal);

            // Compute vertex 1
            size_t vertexIdx = pointIdx * faces * 2 + faceIdx * 2;
            //vertexPosition = DirectX::XMVector3Rotate(vertexPosition, DirectX::XMQuaternionRotationAxis(pointForward, positionAngle));

            DirectX::XMStoreFloat3(&vertices[vertexIdx].position, vertexPosition);
            DirectX::XMStoreFloat3(&vertices[vertexIdx].normal, vertexNormal);
            float txCoordV = pointIdx % 2 == 0 ? 0.0f : 1.0f;
            vertices[vertexIdx].txCoords = {0.0f, txCoordV};

            // Compute vertex 2
            vertexIdx += 1;
            vertexPosition = DirectX::XMVector3Rotate(vertexPosition, DirectX::XMQuaternionRotationAxis(pointForward, angleBetweenFaces));

            DirectX::XMStoreFloat3(&vertices[vertexIdx].position, vertexPosition);
            DirectX::XMStoreFloat3(&vertices[vertexIdx].normal, vertexNormal);
            vertices[vertexIdx].txCoords = {1.0f, txCoordV};
        }
    }

    // Make indices
    std::vector<unsigned int> indices;
    indices.resize(faces * 2 * 3 * (points.size()/2)); // Two triangles per face with 3 vertices each

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
