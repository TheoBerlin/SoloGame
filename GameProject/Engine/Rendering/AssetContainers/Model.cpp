#include "Model.hpp"

Model::Model()
{
}

Model::~Model()
{
}

std::vector<Mesh>& Model::getMeshes()
{
    return this->meshes;
}

std::vector<Material>& Model::getMaterials()
{
    return this->materials;
}

void Model::allocateMeshes(size_t meshCount)
{
    this->meshes.resize(meshCount);
}

void Model::allocateMaterials(size_t materialCount)
{
    this->materials.resize(materialCount);
}
