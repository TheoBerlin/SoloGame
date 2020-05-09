#pragma once

#include <Engine/Rendering/APIAbstractions/GeneralResources.hpp>
#include <Engine/Utils/EnumClass.hpp>

#include <glm/glm.hpp>

enum class TEXTURE_LAYOUT : uint32_t {
    SHADER_READ_ONLY    = 1,
    RENDER_TARGET       = SHADER_READ_ONLY << 1,
    DEPTH_ATTACHMENT    = RENDER_TARGET << 1
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
    Texture(const glm::uvec2& dimensions) :m_Dimensions(dimensions) {}
    virtual ~Texture() = 0 {};

    const glm::uvec2& getDimensions() const { return m_Dimensions; }

    virtual void convertTextureLayout(TEXTURE_LAYOUT oldLayout, TEXTURE_LAYOUT newLayout) = 0;

protected:
    glm::uvec2 m_Dimensions;
};
