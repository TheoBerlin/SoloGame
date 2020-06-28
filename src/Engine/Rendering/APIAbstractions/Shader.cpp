#include "Shader.hpp"

std::string Shader::getTypePostfix(SHADER_TYPE shaderType)
{
    switch (shaderType) {
        case SHADER_TYPE::VERTEX_SHADER:
            return "_vs";
        case SHADER_TYPE::HULL_SHADER:
            return "_hs";
        case SHADER_TYPE::DOMAIN_SHADER:
            return "_ds";
        case SHADER_TYPE::GEOMETRY_SHADER:
            return "_gs";
        case SHADER_TYPE::FRAGMENT_SHADER:
            return "_fs";
        default:
            LOG_ERROR("Erroneous shader type: %d", (int)shaderType);
            return "_vs";
    }
}

Shader::Shader(SHADER_TYPE shaderType)
    :m_ShaderType(shaderType)
{}
