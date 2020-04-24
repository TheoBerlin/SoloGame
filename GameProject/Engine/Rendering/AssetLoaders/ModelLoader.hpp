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

class ShaderResourceHandler;
class TextureLoader;

class ModelLoader : public ComponentHandler
{
public:
    ModelLoader(ECSCore* pECS, TextureLoader* txLoader);
    ~ModelLoader();

    virtual bool initHandler() override;

    Model* loadModel(const std::string& filePath);

    void deleteAllModels();

private:
    /*
        Functions for converting assimp structures to engine's structures
    */
    void loadMesh(const aiMesh* assimpMesh, std::vector<Mesh>& meshes);
    void loadMaterial(const aiMaterial* assimpMaterial, std::vector<Material>& materials, const std::string& directory);

    // Recursively traverse assimp scene nodes to find out which meshes are used
    void loadNode(std::vector<unsigned int>& meshIndices, aiNode* node, const aiScene* scene);

private:
    TextureLoader* m_pTXLoader;
    ShaderResourceHandler* m_pShaderResourceHandler;

    std::unordered_map<std::string, Model*> m_Models;
};
