#pragma once

#include <Engine/Rendering/AssetContainers/Material.hpp>
#include <Engine/Rendering/AssetContainers/Model.hpp>
#include <Engine/Utils/ECSUtils.hpp>
#include <Engine/Utils/IDVector.hpp>

#include <assimp/material.h>
#include <assimp/mesh.h>
#include <assimp/scene.h>

#include <string>
#include <vector>
#include <unordered_map>

class Device;
class TextureCache;

class ModelLoader
{
public:
    ModelLoader(TextureCache* pTextureCache, Device* pDevice);
    ~ModelLoader() = default;

    ModelComponent LoadModel(const std::string& filePath);

private:
    /*
        Functions for converting assimp structures to engine's structures
    */
    void LoadMesh(const aiMesh* pAssimpMesh, std::vector<Mesh>& meshes);
    void LoadMaterial(const aiMaterial* pAssimpMaterial, std::vector<Material>& materials, const std::string& directory);

    // Recursively traverse assimp scene nodes to find out which meshes are used
    void LoadNode(std::vector<unsigned int>& meshIndices, aiNode* pNode, const aiScene* pScene);

private:
    // The same model can be used by multiple entities, hence the weak pointer
    std::unordered_map<std::string, std::weak_ptr<Model>> m_ModelCache;

    TextureCache* m_pTextureCache;
    Device* m_pDevice;
};
