#pragma once

#include <Engine/Rendering/AssetContainers/Material.hpp>
#include <Engine/Rendering/AssetContainers/Mesh.hpp>
#include <assimp/material.h>
#include <assimp/mesh.h>
#include <assimp/scene.h>
#include <string>
#include <vector>
#include <unordered_map>

class Model;

class ModelLoader
{
public:
    ~ModelLoader();

    static Model* loadModel(const std::string& filePath, const std::vector<TX_TYPE>& desiredTextures);

    static void deleteAllModels();

private:
    /*
        Functions for converting assimp structures to engine's structures
    */
    static void loadMesh(const aiMesh* assimpMesh, Mesh& mesh);
    static void loadMaterial(const aiMaterial* assimpMaterial, Material& material, const std::vector<TX_TYPE>& desiredTextures,
    const std::string& directory);

    // Recursively traverse scene nodes to find out which meshes are used
    static void loadNode(std::vector<unsigned int>& meshIndices, aiNode* node, const aiScene* scene);

    static std::unordered_map<std::string, Model*> models;
};
