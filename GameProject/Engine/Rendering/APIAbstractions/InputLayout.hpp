#pragma once

#include <Engine/Rendering/APIAbstractions/GeneralResources.hpp>

#include <string>
#include <vector>

enum class PRIMITIVE_TOPOLOGY {
    POINT_LIST,
    LINE_LIST,
    LINE_STRIP,
    TRIANGLE_LIST,
    TRIANGLE_STRIP,
    LINE_LIST_WITH_ADJACENCY,
    LINE_STRIP_WITH_ADJACENCY,
    TRIANGLE_LIST_WITH_ADJACENCY,
    TRIANGLE_STRIP_WITH_ADJACENCY
};

enum class VERTEX_INPUT_RATE {
    PER_VERTEX,
    PER_INSTANCE
};

// Vertex attributes need to be specified in the order they appear in their buffers
struct InputVertexAttribute {
    std::string SemanticName;
    RESOURCE_FORMAT Format;
};

struct InputLayoutInfo {
    std::vector<InputVertexAttribute> VertexInputAttributes;
    VERTEX_INPUT_RATE InputRate;
    uint32_t Binding;
};
