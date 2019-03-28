#pragma once

#include <Engine/Rendering/AssetContainers/Mesh.hpp>
#include <vector>

class Model
{
public:
    Model();
    ~Model();

    std::vector<Mesh>& getMeshes();
    std::vector<Material>& getMaterials();

    void allocateMeshes(size_t meshCount);
    void allocateMaterials(size_t materialCount);

private:
    std::vector<Mesh> meshes;
    std::vector<Material> materials;
};
