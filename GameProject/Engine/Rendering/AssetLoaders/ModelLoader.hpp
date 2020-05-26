#pragma once

#include <Engine/ECS/ComponentHandler.hpp>
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

const std::type_index g_TIDModel = TID(Model);

class ModelLoader : public ComponentHandler
{
public:
    ModelLoader(ECSCore* pECS, TextureCache* txLoader, Device* pDevice);
    ~ModelLoader() = default;

    virtual bool initHandler() override { return true; };

    Model* loadModel(Entity entity, const std::string& filePath);
    void registerModel(Entity entity, Model* pModel);

    Model* getModel(Entity entity) { return m_ModelComponents.indexID(entity).get(); }

private:
    /*
        Functions for converting assimp structures to engine's structures
    */
    void loadMesh(const aiMesh* assimpMesh, std::vector<Mesh>& meshes);
    void loadMaterial(const aiMaterial* assimpMaterial, std::vector<Material>& materials, const std::string& directory);

    // Recursively traverse assimp scene nodes to find out which meshes are used
    void loadNode(std::vector<unsigned int>& meshIndices, aiNode* node, const aiScene* scene);

private:
    // The same model can be used by multiple entities, hence the shared pointer
    IDDVector<std::shared_ptr<Model>> m_ModelComponents;
    std::unordered_map<std::string, std::weak_ptr<Model>> m_ModelCache;

    TextureCache* m_pTXLoader;
    Device* m_pDevice;
};
