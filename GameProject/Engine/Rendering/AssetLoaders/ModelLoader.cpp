#include "ModelLoader.hpp"

#include <assimp/Importer.hpp>
#include <assimp/postprocess.h>
#include <assimp/scene.h>
#include <Engine/Rendering/AssetContainers/Model.hpp>
#include <Engine/Rendering/AssetLoaders/TextureLoader.hpp>
#include <Engine/Utils/Logger.hpp>

std::unordered_map<std::string, Model*> ModelLoader::models = std::unordered_map<std::string, Model*>();

ModelLoader::~ModelLoader()
{
    ModelLoader::deleteAllModels();
}


Model* ModelLoader::loadModel(const std::string& filePath, const std::vector<aiTextureType>& desiredTextures)
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

    const aiScene* scene = importer.ReadFile(filePath, aiProcess_Triangulate | aiProcess_JoinIdenticalVertices | aiProcess_GenNormals | aiProcess_FlipUVs);

    if (!scene) {
        Logger::LOG_WARNING("Model could not be loaded: [%s]", filePath.c_str());
        return nullptr;
    } else {
        Logger::LOG_INFO("Loading model [%s] containing [%d] meshes", filePath.c_str(), scene->mNumMeshes);
    }

    Model* loadedModel = new Model();

    // Load meshes
    loadedModel->allocateMeshes(scene->mNumMeshes);
    std::vector<Mesh>& meshes = loadedModel->getMeshes();

    for (unsigned int i = 0; i < scene->mNumMeshes; i += 1) {
        loadMesh(scene->mMeshes[i], meshes[i]);
    }

    // Load materials
    loadedModel->allocateMaterials(scene->mNumMaterials);
    std::vector<Material>& materials = loadedModel->getMaterials();

    for (unsigned int i = 0; i < scene->mNumMaterials; i += 1) {
        loadMaterial(scene->mMaterials[i], materials[i], desiredTextures, directory);
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

void ModelLoader::loadMesh(const aiMesh* assimpMesh, Mesh& mesh)
{
    if (!assimpMesh->HasPositions() || !assimpMesh->HasNormals() || !assimpMesh->HasTextureCoords(0)) {
        Logger::LOG_WARNING("Assimp mesh is missing expected data");
        return;
    }

    mesh.allocateVertices(assimpMesh->mNumVertices);

    std::vector<Vertex>& vertices = mesh.getVertices();

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

    mesh.setMaterialIndex(assimpMesh->mMaterialIndex);
}

void ModelLoader::loadMaterial(const aiMaterial* assimpMaterial, Material& material, const std::vector<aiTextureType>& desiredTextures,
    const std::string& directory)
{
    for (size_t i = 0; i < desiredTextures.size(); i += 1) {
        aiString textureName;

        if (assimpMaterial->GetTextureCount((aiTextureType)desiredTextures[i]) == 0) {
            continue;
        }

        // Get file path of a desired texture
        assimpMaterial->GetTexture((aiTextureType)desiredTextures[i], 0, &textureName);

        std::string texturePath = directory + std::string(textureName.C_Str());

        Logger::LOG_INFO("Loading texture [%s]", texturePath.c_str());

        material.textures.push_back(TextureLoader::loadTexture(texturePath));
    }

    aiColor3D aiDiffuse, aiSpecular;

    assimpMaterial->Get(AI_MATKEY_COLOR_DIFFUSE, aiDiffuse);
    assimpMaterial->Get(AI_MATKEY_COLOR_DIFFUSE, aiSpecular);

    material.diffuse = DirectX::XMFLOAT4(aiDiffuse.r, aiDiffuse.g, aiDiffuse.b, 1.0f);
    material.specular = DirectX::XMFLOAT4(aiSpecular.r, aiSpecular.g, aiSpecular.b, 1.0f);
}
