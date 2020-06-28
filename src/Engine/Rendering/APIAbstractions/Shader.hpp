#pragma once

#include <Engine/Utils/EnumClass.hpp>

#include <stdint.h>

#define SHADERS_FOLDER_PATH "assets/Engine/Shaders/"

enum class SHADER_TYPE : uint32_t {
    VERTEX_SHADER   = 1,
    HULL_SHADER     = VERTEX_SHADER << 1,
    DOMAIN_SHADER   = HULL_SHADER   << 1,
    GEOMETRY_SHADER = DOMAIN_SHADER << 1,
    FRAGMENT_SHADER = GEOMETRY_SHADER << 1
};

DEFINE_BITMASK_OPERATIONS(SHADER_TYPE)

class Shader
{
public:
    static std::string getTypePostfix(SHADER_TYPE shaderType);

public:
    Shader(SHADER_TYPE shaderType);
    virtual ~Shader() = 0 {};

    inline SHADER_TYPE getShaderType() const { return m_ShaderType; }

private:
    SHADER_TYPE m_ShaderType;
};
