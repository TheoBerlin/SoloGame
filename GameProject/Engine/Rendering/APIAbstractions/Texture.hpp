#pragma once

#include <Engine/Rendering/APIAbstractions/GeneralResources.hpp>

#include <glm/glm.hpp>

enum class TEXTURE_LAYOUT : uint32_t {
    UNDEFINED           = 1,
    SHADER_READ_ONLY    = UNDEFINED << 1,
    RENDER_TARGET       = SHADER_READ_ONLY << 1,
    DEPTH_ATTACHMENT    = RENDER_TARGET << 1,
    PRESENT             = DEPTH_ATTACHMENT << 1
};

DEFINE_BITMASK_OPERATIONS(TEXTURE_LAYOUT)

struct InitialData {
    const void* pData;
    uint32_t RowSize;   // Size of a row in the texture in bytes
};

struct TextureInfo {
    glm::uvec2 Dimensions;
    TEXTURE_LAYOUT LayoutFlags; // Only used by DX11
    TEXTURE_LAYOUT InitialLayout;
    RESOURCE_FORMAT Format;
    InitialData* pInitialData;  // Optional
};

class Texture
{
public:
    Texture(const glm::uvec2& dimensions, RESOURCE_FORMAT format) :m_Dimensions(dimensions), m_Format(format) {}
    virtual ~Texture() = 0 {};

    const glm::uvec2& getDimensions() const { return m_Dimensions; }
    inline RESOURCE_FORMAT getFormat() const { return m_Format; }

protected:
    glm::uvec2 m_Dimensions;
    RESOURCE_FORMAT m_Format;
};
