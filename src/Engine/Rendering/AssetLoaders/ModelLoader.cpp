#include "ModelLoader.hpp"

#include <Engine/ECS/ECSCore.hpp>
#include <Engine/Rendering/APIAbstractions/Device.hpp>
#include <Engine/Rendering/APIAbstractions/IBuffer.hpp>
#include <Engine/Rendering/AssetContainers/Model.hpp>
#include <Engine/Rendering/AssetLoaders/TextureCache.hpp>
#include <Engine/Utils/Debug.hpp>
#include <Engine/Utils/DirectXUtils.hpp>
#include <Engine/Utils/ECSUtils.hpp>

#include <vendor/assimp/Importer.hpp>
#include <vendor/assimp/postprocess.h>

#include <algorithm>

ModelLoader::ModelLoader(TextureCache* pTextureCache, Device* pDevice)
    :m_pTextureCache(pTextureCache),
    m_pDevice(pDevice)
{}

ModelComponent ModelLoader::LoadModel(const std::string& filePath)
{
    auto itr = m_ModelCache.find(filePath);
    if (itr != m_ModelCache.end()) {
        std::weak_ptr<Model>& modelPtr = itr->second;

        if (modelPtr.expired()) {
            // The model used to exist but has been deleted
            m_ModelCache.erase(itr);
        } else {
            return ModelComponent{
                .ModelPtr = modelPtr.lock()
            };
        }
    }

    // Get the directory path
    size_t lastDivider = filePath.find_last_of('/');

    if (lastDivider == std::string::npos) {
        lastDivider = filePath.find_last_of('\\');
    }

    std::string directory = "./";

    // Default to 0 if no divider was found
    if (lastDivider != std::string::npos) {
        directory = filePath.substr(0, lastDivider) + "/";
    }

    // Load the model
    Assimp::Importer importer;
    const aiScene* pScene = importer.ReadFile(filePath, aiProcess_GenUVCoords | aiProcess_Triangulate | aiProcess_JoinIdenticalVertices | aiProcess_GenNormals | aiProcess_FlipUVs);

    if (!pScene) {
        LOG_WARNINGF("Model could not be loaded: [%s]", filePath.c_str());
        return { };
    } else {
        LOG_INFOF("Loading model [%s] containing [%d] meshes and [%d] materials", filePath.c_str(), pScene->mNumMeshes, pScene->mNumMaterials);
    }

    ModelComponent modelComponent = {
        .ModelPtr = std::shared_ptr<Model>(DBG_NEW Model(), ReleaseModel)
    };

    Model* pModel = modelComponent.ModelPtr.get();

    // Traverse nodes to find which meshes to load
    std::vector<unsigned int> meshIndices;
    LoadNode(meshIndices, pScene->mRootNode, pScene);

    // Load meshes
    pModel->Meshes.reserve(meshIndices.size());

    for (unsigned int meshIndex : meshIndices) {
        LoadMesh(pScene->mMeshes[meshIndex], pModel->Meshes);
    }

    // Load materials
    pModel->Materials.reserve(pScene->mNumMaterials);

    for (unsigned int i = 0; i < pScene->mNumMaterials; i += 1) {
        LoadMaterial(pScene->mMaterials[i], pModel->Materials, directory);
    }

    // Not all materials might have been loaded, subtract the material indices to compensate
    size_t materialIndexOffset = pScene->mNumMaterials - pModel->Materials.size();
    if (materialIndexOffset > 0) {
        for (Mesh& mesh : pModel->Meshes) {
            mesh.materialIndex -= materialIndexOffset;
        }
    }

    m_ModelCache[filePath] = modelComponent.ModelPtr;
    return modelComponent;
}

void ModelLoader::LoadMesh(const aiMesh* pAssimpMesh, std::vector<Mesh>& meshes)
{
    if (!pAssimpMesh->HasPositions()) {
        LOG_WARNING("Assimp mesh is missing vertex positions");
        return;
    }

    if (!pAssimpMesh->HasNormals()) {
        LOG_WARNING("Assimp mesh is missing normals");
        return;
    }

    Mesh mesh;
    mesh.materialIndex = pAssimpMesh->mMaterialIndex;

    std::vector<Vertex> vertices;
    vertices.resize(pAssimpMesh->mNumVertices);

    // Read vertex data
    for (unsigned int i = 0; i < pAssimpMesh->mNumVertices; i += 1) {
        vertices[i].position.x = pAssimpMesh->mVertices[i].x;
        vertices[i].position.y = pAssimpMesh->mVertices[i].y;
        vertices[i].position.z = pAssimpMesh->mVertices[i].z;

        vertices[i].normal.x = pAssimpMesh->mNormals[i].x;
        vertices[i].normal.y = pAssimpMesh->mNormals[i].y;
        vertices[i].normal.z = pAssimpMesh->mNormals[i].z;

        vertices[i].txCoords.x = pAssimpMesh->mTextureCoords[0][i].x;
        vertices[i].txCoords.y = pAssimpMesh->mTextureCoords[0][i].y;
    }

    mesh.vertexCount = vertices.size();

    // Read indices
    std::vector<unsigned int> indices;
    indices.resize(size_t(pAssimpMesh->mNumFaces) * 3u);

    for (size_t faceIdx = 0; faceIdx < pAssimpMesh->mNumFaces; faceIdx += 1) {
        const aiFace* face = &pAssimpMesh->mFaces[faceIdx];

        if (face->mNumIndices != 3) {
            LOG_WARNINGF("Mesh face has an unexpected amount of indices: %d", face->mNumIndices);
            return;
        }

        indices[faceIdx * 3]        = face->mIndices[0];
        indices[faceIdx * 3 + 1]    = face->mIndices[1];
        indices[faceIdx * 3 + 2]    = face->mIndices[2];
    }

    mesh.indexCount = indices.size();

    mesh.pVertexBuffer = m_pDevice->createVertexBuffer(&vertices.front(), sizeof(Vertex), vertices.size());
    if (!mesh.pVertexBuffer) {
        return;
    }

    mesh.pIndexBuffer = m_pDevice->createIndexBuffer(&indices.front(), indices.size());
    if (!mesh.pIndexBuffer) {
        return;
    }

    meshes.push_back(mesh);
}

void ModelLoader::LoadMaterial(const aiMaterial* pAssimpMaterial, std::vector<Material>& materials, const std::string& directory)
{
    Material material;

    // Get diffuse texture
    if (pAssimpMaterial->GetTextureCount(aiTextureType_DIFFUSE) == 0) {
        LOG_WARNING("Loading material lacks a diffuse texture");
        return;
    }

    // Get file path of a desired texture
    aiString textureName;
    pAssimpMaterial->GetTexture(aiTextureType_DIFFUSE, 0, &textureName);

    std::string textureNameStd = textureName.C_Str();

    // Remove all whitespaces from the texture name
    std::replace(textureNameStd.begin(), textureNameStd.end(), ' ', '_');

    std::string texturePath = directory + textureNameStd;

    material.textures.push_back(m_pTextureCache->LoadTexture(texturePath));

    // Load material attributes
    aiColor3D aiAmbient, aiSpecular;
    ai_real aiShininess, aiShininessStrength;

    //assimpMaterial->Get(AI_MATKEY_COLOR_AMBIENT, aiAmbient);
    pAssimpMaterial->Get(AI_MATKEY_COLOR_DIFFUSE, aiSpecular);
    pAssimpMaterial->Get(AI_MATKEY_SHININESS, aiShininess);
    pAssimpMaterial->Get(AI_MATKEY_SHININESS_STRENGTH, aiShininessStrength);

    // Set shininess factor to 0.5 if there is none
    aiShininessStrength = aiShininessStrength < 0.01f || aiShininessStrength > 1.0f ? 0.5f : aiShininessStrength;

    //material.attributes.ambient = DirectX::XMFLOAT3(aiAmbient.r, aiAmbient.g, aiAmbient.b);
    material.attributes.specular = DirectX::XMFLOAT4(aiShininess, aiShininessStrength, 0.0f, 0.0f);

    materials.push_back(material);
}

void ModelLoader::LoadNode(std::vector<unsigned int>& meshIndices, aiNode* pNode, const aiScene* pScene)
{
    meshIndices.reserve(meshIndices.size() + pNode->mNumMeshes);

    // Insert this node's mesh indices
    for (unsigned int i = 0; i < pNode->mNumMeshes; i += 1) {

        // Make sure the mesh has texture coordinates
        if (!pScene->mMeshes[pNode->mMeshes[i]]->HasTextureCoords(0)) {
            LOG_WARNINGF("Ignoring mesh [%d]: missing texture coordinates", pNode->mMeshes[i]);
            continue;
        }

        // Make sure the mesh index is not already listed
        bool alreadyExists = false;

        for (size_t j = 0; j < meshIndices.size() && !alreadyExists; j += 1) {
            if (meshIndices[j] == pNode->mMeshes[i]) {
                alreadyExists = true;
            }
        }

        if (!alreadyExists) {
            meshIndices.push_back(pNode->mMeshes[i]);
        }
    }

    // Insert childrens' mesh indices
    for (unsigned int i = 0; i < pNode->mNumChildren; i += 1) {
        LoadNode(meshIndices, pNode->mChildren[i], pScene);
    }
}
