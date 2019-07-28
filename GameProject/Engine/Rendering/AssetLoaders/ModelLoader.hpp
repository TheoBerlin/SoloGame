#pragma once

#include <Engine/ECS/ComponentHandler.hpp>
#include <Engine/Rendering/AssetContainers/Material.hpp>
#include <Engine/Rendering/AssetContainers/Model.hpp>
#include <assimp/material.h>
#include <assimp/mesh.h>
#include <assimp/scene.h>
#include <d3d11.h>
#include <string>
#include <vector>
#include <unordered_map>

class TextureLoader;

class ModelLoader : public ComponentHandler
{
public:
    ModelLoader(SystemSubscriber* sysSubscriber, TextureLoader* txLoader, ID3D11Device* device);
    ~ModelLoader();

    Model* loadModel(const std::string& filePath);

    void deleteAllModels();

private:
    /*
        Functions for converting assimp structures to engine's structures
    */
    void loadMesh(const aiMesh* assimpMesh, std::vector<Mesh>& meshes);
    void loadMaterial(const aiMaterial* assimpMaterial, std::vector<Material>& materials, const std::string& directory);

    ID3D11Device* device;
    TextureLoader* txLoader;

    // Recursively traverse scene nodes to find out which meshes are used
    void loadNode(std::vector<unsigned int>& meshIndices, aiNode* node, const aiScene* scene);

    std::unordered_map<std::string, Model*> models;
};
