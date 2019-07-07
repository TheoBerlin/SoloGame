#include "ModelLoader.hpp"

#include <assimp/Importer.hpp>
#include <assimp/postprocess.h>
#include <Engine/Rendering/AssetContainers/Model.hpp>
#include <Engine/Rendering/AssetLoaders/TextureLoader.hpp>
#include <Engine/Utils/Logger.hpp>
#include <algorithm>

std::unordered_map<std::string, Model*> ModelLoader::models = std::unordered_map<std::string, Model*>();

ModelLoader::~ModelLoader()
{
    ModelLoader::deleteAllModels();
}


Model* ModelLoader::loadModel(const std::string& filePath, const std::vector<TX_TYPE>& desiredTextures)
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
        Logger::LOG_INFO("Loading model [%s] containing [%d] meshes", filePath.c_str(), scene->mNumMeshes);
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
        loadMaterial(scene->mMaterials[i], loadedModel->materials, desiredTextures, directory);
    }

    // Store a pointer to the loaded model
    models[filePath] = loadedModel;

    return loadedModel;
}

void ModelLoader::deleteAllModels()
{
    for (auto& itr : models) {
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

    mesh.vertices.resize(assimpMesh->mNumVertices);

    std::vector<Vertex>& vertices = mesh.vertices;

    // Loop through vertex data
    for (unsigned int i = 0; i < assimpMesh->mNumVertices; i += 1) {
        vertices[i].position.x = assimpMesh->mVertices->x;
        vertices[i].position.y = assimpMesh->mVertices->y;
        vertices[i].position.z = assimpMesh->mVertices->z;

        vertices[i].normal.x = assimpMesh->mNormals->x;
        vertices[i].normal.y = assimpMesh->mNormals->y;
        vertices[i].normal.z = assimpMesh->mNormals->z;

        vertices[i].txCoords.x = assimpMesh->mTextureCoords[0][i].x;
        vertices[i].txCoords.y = assimpMesh->mTextureCoords[0][i].y;
    }

    mesh.materialIndex = assimpMesh->mMaterialIndex;

    meshes.push_back(mesh);
}

void ModelLoader::loadMaterial(const aiMaterial* assimpMaterial, std::vector<Material>& materials, const std::vector<TX_TYPE>& desiredTextures,
    const std::string& directory)
{
    Material material;

    for (size_t i = 0; i < desiredTextures.size(); i += 1) {
        aiString textureName;

        if (desiredTextures[i] == TX_TYPE::NO_TEXTURE ||
        assimpMaterial->GetTextureCount((aiTextureType)desiredTextures[i]) == 0) {
            continue;
        }

        // Get file path of a desired texture
        assimpMaterial->GetTexture((aiTextureType)desiredTextures[i], 0, &textureName);

        std::string textureNameStd = textureName.C_Str();

        // Remove all whitespaces from the texture name
        std::replace(textureNameStd.begin(), textureNameStd.end(), ' ', '_');

        std::string texturePath = directory + textureNameStd;

        material.textures.push_back(Texture(texturePath, desiredTextures[i]));
    }

    aiColor3D aiDiffuse, aiSpecular;

    assimpMaterial->Get(AI_MATKEY_COLOR_DIFFUSE, aiDiffuse);
    assimpMaterial->Get(AI_MATKEY_COLOR_DIFFUSE, aiSpecular);

    material.diffuse = DirectX::XMFLOAT4(aiDiffuse.r, aiDiffuse.g, aiDiffuse.b, 1.0f);
    material.specular = DirectX::XMFLOAT4(aiSpecular.r, aiSpecular.g, aiSpecular.b, 1.0f);

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
