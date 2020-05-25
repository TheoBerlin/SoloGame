#include "ModelLoader.hpp"

#define NOMINMAX

#include <Engine/ECS/ECSCore.hpp>
#include <Engine/Rendering/APIAbstractions/Device.hpp>
#include <Engine/Rendering/APIAbstractions/IBuffer.hpp>
#include <Engine/Rendering/AssetContainers/Model.hpp>
#include <Engine/Rendering/AssetLoaders/TextureCache.hpp>
#include <Engine/Utils/DirectXUtils.hpp>
#include <Engine/Utils/ECSUtils.hpp>
#include <Engine/Utils/Logger.hpp>

#include <assimp/Importer.hpp>
#include <assimp/postprocess.h>

#include <algorithm>

ModelLoader::ModelLoader(ECSCore* pECS, TextureCache* txLoader, Device* pDevice)
    :ComponentHandler(pECS, TID(ModelLoader)),
    m_pTXLoader(txLoader),
    m_pDevice(pDevice)
{
    ComponentHandlerRegistration handlerReg = {};
    handlerReg.pComponentHandler = this;
    handlerReg.ComponentRegistrations = {
        {{g_TIDModel}, &m_ModelComponents}
    };
    registerHandler(handlerReg);
}

Model* ModelLoader::loadModel(Entity entity, const std::string& filePath)
{
    auto itr = m_ModelCache.find(filePath);
    if (itr != m_ModelCache.end()) {
        std::weak_ptr<Model>& modelPtr = itr->second;

        if (modelPtr.expired()) {
            // The texture used to exist but has been deleted
            m_ModelCache.erase(itr);
        } else {
            return modelPtr.lock().get();
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
    const aiScene* scene = importer.ReadFile(filePath, aiProcess_GenUVCoords | aiProcess_Triangulate | aiProcess_JoinIdenticalVertices | aiProcess_GenNormals | aiProcess_FlipUVs);

    if (!scene) {
        LOG_WARNING("Model could not be loaded: [%s]", filePath.c_str());
        return nullptr;
    } else {
        LOG_INFO("Loading model [%s] containing [%d] meshes and [%d] materials", filePath.c_str(), scene->mNumMeshes, scene->mNumMaterials);
    }

    std::shared_ptr<Model> pLoadedModel = std::shared_ptr<Model>(new Model(), releaseModel);

    // Traverse nodes to find which meshes to load
    std::vector<unsigned int> meshIndices;
    loadNode(meshIndices, scene->mRootNode, scene);

    // Load meshes
    pLoadedModel->Meshes.reserve(meshIndices.size());

    for (unsigned int meshIndex : meshIndices) {
        loadMesh(scene->mMeshes[meshIndex], pLoadedModel->Meshes);
    }

    // Load materials
    pLoadedModel->Materials.reserve(scene->mNumMaterials);

    for (unsigned int i = 0; i < scene->mNumMaterials; i += 1) {
        loadMaterial(scene->mMaterials[i], pLoadedModel->Materials, directory);
    }

    // Not all materials might have been loaded, subtract the material indices to compensate
    size_t materialIndexOffset = scene->mNumMaterials - pLoadedModel->Materials.size();
    if (materialIndexOffset > 0) {
        for (Mesh& mesh : pLoadedModel->Meshes) {
            mesh.materialIndex -= materialIndexOffset;
        }
    }

    // Map the model for caching and store it as a component
    m_ModelCache[filePath] = pLoadedModel;
    m_ModelComponents.push_back(pLoadedModel, entity);
    registerComponent(entity, g_TIDModel);

    return pLoadedModel.get();
}

void ModelLoader::registerModel(Entity entity, Model* pModel)
{
    m_ModelComponents.push_back(std::shared_ptr<Model>(pModel, releaseModel), entity);
    registerComponent(entity, g_TIDModel);
}

void ModelLoader::loadMesh(const aiMesh* assimpMesh, std::vector<Mesh>& meshes)
{
    if (!assimpMesh->HasPositions()) {
        LOG_WARNING("Assimp mesh is missing vertex positions");
        return;
    }

    if (!assimpMesh->HasNormals()) {
        LOG_WARNING("Assimp mesh is missing normals");
        return;
    }

    Mesh mesh;
    mesh.materialIndex = assimpMesh->mMaterialIndex;

    std::vector<Vertex> vertices;
    vertices.resize(assimpMesh->mNumVertices);

    // Read vertex data
    for (unsigned int i = 0; i < assimpMesh->mNumVertices; i += 1) {
        vertices[i].position.x = assimpMesh->mVertices[i].x;
        vertices[i].position.y = assimpMesh->mVertices[i].y;
        vertices[i].position.z = assimpMesh->mVertices[i].z;

        vertices[i].normal.x = assimpMesh->mNormals[i].x;
        vertices[i].normal.y = assimpMesh->mNormals[i].y;
        vertices[i].normal.z = assimpMesh->mNormals[i].z;

        vertices[i].txCoords.x = assimpMesh->mTextureCoords[0][i].x;
        vertices[i].txCoords.y = assimpMesh->mTextureCoords[0][i].y;
    }

    mesh.vertexCount = vertices.size();

    // Read indices
    std::vector<unsigned int> indices;
    indices.resize(assimpMesh->mNumFaces * 3);

    for (unsigned int i = 0; i < assimpMesh->mNumFaces; i += 1) {
        const aiFace* face = &assimpMesh->mFaces[i];

        if (face->mNumIndices != 3) {
            LOG_WARNING("Mesh face has an unexpected amount of indices: %d", face->mNumIndices);
            return;
        }

        indices[i * 3] = face->mIndices[0];
        indices[i * 3 + 1] = face->mIndices[1];
        indices[i * 3 + 2] = face->mIndices[2];
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

void ModelLoader::loadMaterial(const aiMaterial* assimpMaterial, std::vector<Material>& materials, const std::string& directory)
{
    Material material;

    // Get diffuse texture
    aiString textureName;

    if (assimpMaterial->GetTextureCount(aiTextureType_DIFFUSE) == 0) {
        LOG_WARNING("Loading material lacks a diffuse texture");
        return;
    }

    // Get file path of a desired texture
    assimpMaterial->GetTexture(aiTextureType_DIFFUSE, 0, &textureName);

    std::string textureNameStd = textureName.C_Str();

    // Remove all whitespaces from the texture name
    std::replace(textureNameStd.begin(), textureNameStd.end(), ' ', '_');

    std::string texturePath = directory + textureNameStd;

    material.textures.push_back(m_pTXLoader->loadTexture(texturePath));

    // Load material attributes
    aiColor3D aiAmbient, aiSpecular;
    ai_real aiShininess, aiShininessStrength;

    //assimpMaterial->Get(AI_MATKEY_COLOR_AMBIENT, aiAmbient);
    assimpMaterial->Get(AI_MATKEY_COLOR_DIFFUSE, aiSpecular);
    assimpMaterial->Get(AI_MATKEY_SHININESS, aiShininess);
    assimpMaterial->Get(AI_MATKEY_SHININESS_STRENGTH, aiShininessStrength);

    // Set shininess factor to 0.5 if there is none
    aiShininessStrength = aiShininessStrength < 0.01f || aiShininessStrength > 1.0f ? 0.5f : aiShininessStrength;

    //material.attributes.ambient = DirectX::XMFLOAT3(aiAmbient.r, aiAmbient.g, aiAmbient.b);
    material.attributes.specular = DirectX::XMFLOAT4(aiShininess, aiShininessStrength, 0.0f, 0.0f);

    materials.push_back(material);
}

void ModelLoader::loadNode(std::vector<unsigned int>& meshIndices, aiNode* node, const aiScene* scene)
{
    meshIndices.reserve(meshIndices.size() + node->mNumMeshes);

    // Insert this node's mesh indices
    for (unsigned int i = 0; i < node->mNumMeshes; i += 1) {

        // Make sure the mesh has texture coordinates
        if (!scene->mMeshes[node->mMeshes[i]]->HasTextureCoords(0)) {
            LOG_WARNING("Ignoring mesh [%d]: missing texture coordinates", node->mMeshes[i]);
            continue;
        }

        // Make sure the mesh index is not already listed
        bool alreadyExists = false;

        for (size_t j = 0; j < meshIndices.size() && !alreadyExists; j += 1) {
            if (meshIndices[j] == node->mMeshes[i]) {
                alreadyExists = true;
            }
        }

        if (!alreadyExists) {
            meshIndices.push_back(node->mMeshes[i]);
        }
    }

    // Insert childrens' mesh indices
    for (unsigned int i = 0; i < node->mNumChildren; i += 1) {
        loadNode(meshIndices, node->mChildren[i], scene);
    }
}
