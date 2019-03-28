#pragma once

#include <Engine/Rendering/AssetContainers/AssetResources.hpp>
#include <Engine/Rendering/AssetContainers/Mesh.hpp>
#include <assimp/material.h>
#include <assimp/mesh.h>
#include <string>
#include <vector>
#include <unordered_map>

class Model;

class ModelLoader
{
public:
    ~ModelLoader();

    static Model* loadModel(const std::string& filePath, const std::vector<aiTextureType>& desiredTextures);

    static void deleteAllModels();

private:
    /*
        Functions for converting assimp structures to engine's structures
    */
    static void loadMesh(const aiMesh* assimpMesh, Mesh& mesh);
    static void loadMaterial(const aiMaterial* assimpMaterial, Material& material, const std::vector<aiTextureType>& desiredTextures,
    const std::string& directory);

    static std::unordered_map<std::string, Model*> models;
};
