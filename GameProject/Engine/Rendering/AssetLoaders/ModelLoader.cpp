#include "ModelLoader.hpp"

#include <assimp/Importer.hpp>
#include <assimp/postprocess.h>
#include <Engine/Rendering/AssetContainers/Model.hpp>
#include <Engine/Rendering/AssetLoaders/TextureLoader.hpp>
#include <Engine/Utils/DirectXUtils.hpp>
#include <Engine/Utils/Logger.hpp>
#include <algorithm>

ModelLoader::ModelLoader(SystemSubscriber* sysSubscriber, TextureLoader* txLoader, ID3D11Device* device)
    :device(device),
    txLoader(txLoader),
    ComponentHandler({}, sysSubscriber, std::type_index(typeid(ModelLoader)))
{}

ModelLoader::~ModelLoader()
{
    ModelLoader::deleteAllModels();
}

Model* ModelLoader::loadModel(const std::string& filePath)
{
    // See if the model is already loaded
    auto itr = models.find(filePath);

    if (itr != models.end()) {
        // The model was found, return it
        return itr->second;
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
        Logger::LOG_WARNING("Model could not be loaded: [%s]", filePath.c_str());
        return nullptr;
    } else {
        Logger::LOG_INFO("Loading model [%s] containing [%d] meshes and [%d] materials", filePath.c_str(), scene->mNumMeshes, scene->mNumMaterials);
    }

    Model* loadedModel = new Model();

    // Traverse nodes to find which meshes to load
    std::vector<unsigned int> meshIndices;
    loadNode(meshIndices, scene->mRootNode, scene);

    // Load meshes
    loadedModel->meshes.reserve(meshIndices.size());

    for (unsigned int i = 0; i < meshIndices.size(); i += 1) {
        loadMesh(scene->mMeshes[meshIndices[i]], loadedModel->meshes);
    }

    // Load materials
    loadedModel->materials.reserve(scene->mNumMaterials);

    for (unsigned int i = 0; i < scene->mNumMaterials; i += 1) {
        loadMaterial(scene->mMaterials[i], loadedModel->materials, directory);
    }

    // Not all materials might have been loaded, subtract the material indices to compensate
    size_t materialIndexOffset = scene->mNumMaterials - loadedModel->materials.size();
    if (materialIndexOffset > 0) {
        for (size_t i = 0; i < loadedModel->meshes.size(); i += 1) {
            loadedModel->meshes[i].materialIndex -= materialIndexOffset;
        }
    }

    // Store a pointer to the loaded model
    models[filePath] = loadedModel;

    return loadedModel;
}

void ModelLoader::deleteAllModels()
{
    for (auto& itr : models) {
        Model* model = itr.second;
        for (size_t i = 0; i < model->meshes.size(); i += 1) {
            model->meshes[i].vertexBuffer->Release();
            model->meshes[i].indexBuffer->Release();
        }
        delete itr.second;
    }

    models.clear();
}

void ModelLoader::loadMesh(const aiMesh* assimpMesh, std::vector<Mesh>& meshes)
{
    if (!assimpMesh->HasPositions()) {
        Logger::LOG_WARNING("Assimp mesh is missing vertex positions");
        return;
    }

    if (!assimpMesh->HasNormals()) {
        Logger::LOG_WARNING("Assimp mesh is missing normals");
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
            Logger::LOG_WARNING("Mesh face has an unexpected amount of indices: %d", face->mNumIndices);
            return;
        }

        indices[i*3] = face->mIndices[0];
        indices[i*3+1] = face->mIndices[1];
        indices[i*3+2] = face->mIndices[2];
    }

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
    }

    meshes.push_back(mesh);
}

void ModelLoader::loadMaterial(const aiMaterial* assimpMaterial, std::vector<Material>& materials, const std::string& directory)
{
    Material material;

    // Get diffuse texture
    aiString textureName;

    if (assimpMaterial->GetTextureCount(aiTextureType_DIFFUSE) == 0) {
        Logger::LOG_WARNING("Loading material lacks a diffuse texture");
        return;
    }

    // Get file path of a desired texture
    assimpMaterial->GetTexture(aiTextureType_DIFFUSE, 0, &textureName);

    std::string textureNameStd = textureName.C_Str();

    // Remove all whitespaces from the texture name
    std::replace(textureNameStd.begin(), textureNameStd.end(), ' ', '_');

    std::string texturePath = directory + textureNameStd;

    material.textures.push_back(txLoader->loadTexture(texturePath, TX_TYPE::DIFFUSE));

    // Load material attributes
    aiColor3D aiAmbient, aiSpecular;
    ai_real aiShininess;

    assimpMaterial->Get(AI_MATKEY_COLOR_AMBIENT, aiAmbient);
    assimpMaterial->Get(AI_MATKEY_COLOR_DIFFUSE, aiSpecular);
    assimpMaterial->Get(AI_MATKEY_SHININESS, aiShininess);

    material.attributes.ambient = DirectX::XMFLOAT3(aiAmbient.r, aiAmbient.g, aiAmbient.b);
    material.attributes.specular = DirectX::XMFLOAT3(aiSpecular.r, aiSpecular.g, aiSpecular.b);

    materials.push_back(material);
}

void ModelLoader::loadNode(std::vector<unsigned int>& meshIndices, aiNode* node, const aiScene* scene)
{
    meshIndices.reserve(meshIndices.size() + node->mNumMeshes);

    // Insert this node's mesh indices
    for (unsigned int i = 0; i < node->mNumMeshes; i += 1) {

        // Make sure the mesh has texture coordinates
        if (!scene->mMeshes[node->mMeshes[i]]->HasTextureCoords(0)) {
            Logger::LOG_WARNING("Ignoring mesh [%d]: missing texture coordinates", node->mMeshes[i]);
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
