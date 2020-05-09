#pragma once

#include <Engine/Rendering/APIAbstractions/ResourceFormat.hpp>

#include <string>
#include <vector>

enum class VERTEX_INPUT_RATE {
    PER_VERTEX,
    PER_INSTANCE
};

// Vertex attributes need to be specified in the order they appear in their buffers
struct InputVertexAttribute {
    std::string SemanticName;
    RESOURCE_FORMAT Format;
    VERTEX_INPUT_RATE InputRate;
};

struct InputLayoutInfo {
    std::vector<InputVertexAttribute> VertexInputAttributes;
    uint32_t Binding;
};

class InputLayout
{
public:
    InputLayout(uint32_t vertexSize) : m_VertexSize(vertexSize) {};
    virtual ~InputLayout() = 0 {};

    inline uint32_t getVertexSize() const { return m_VertexSize; }

private:
    uint32_t m_VertexSize;
};
