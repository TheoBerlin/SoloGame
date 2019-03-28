#include "Mesh.hpp"

Mesh::Mesh()
{
}

Mesh::~Mesh()
{
}

std::vector<Vertex>& Mesh::getVertices()
{
    return this->vertices;
}

void Mesh::allocateVertices(size_t vertexCount)
{
    this->vertices.resize(vertexCount);
}

unsigned int Mesh::getMaterialIndex() const
{
    return this->materialIndex;
}

void Mesh::setMaterialIndex(unsigned int materialIndex)
{
    this->materialIndex = materialIndex;
}
