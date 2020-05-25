#include "ShaderHandler.hpp"

#include <Engine/Rendering/APIAbstractions/Device.hpp>
#include <Engine/Rendering/APIAbstractions/InputLayout.hpp>
#include <Engine/Utils/ECSUtils.hpp>

ShaderHandler::ShaderHandler(Device* pDevice)
    :m_pDevice(pDevice)
{
    // Create and map all input layout infos
    InputLayoutInfo inputLayoutInfo = {};
    inputLayoutInfo.Binding = 0;
    inputLayoutInfo.VertexInputAttributes = {
        {
            "POSITION",
            RESOURCE_FORMAT::R32G32B32_FLOAT,
            VERTEX_INPUT_RATE::PER_VERTEX
        },
        {
            "NORMAL",
            RESOURCE_FORMAT::R32G32B32_FLOAT,
            VERTEX_INPUT_RATE::PER_VERTEX
        },
        {
            "TEXCOORD",
            RESOURCE_FORMAT::R32G32_FLOAT,
            VERTEX_INPUT_RATE::PER_VERTEX
        }
    };

    m_InputLayoutInfos["Mesh"] = inputLayoutInfo;

    // Compile UI program
    inputLayoutInfo.VertexInputAttributes = {
        {
            "POSITION",
            RESOURCE_FORMAT::R32G32_FLOAT,
            VERTEX_INPUT_RATE::PER_VERTEX
        },
        {
            "TEXCOORD",
            RESOURCE_FORMAT::R32G32_FLOAT,
            VERTEX_INPUT_RATE::PER_VERTEX
        }
    };

    m_InputLayoutInfos["UI"] = inputLayoutInfo;
}

std::shared_ptr<VertexStage> ShaderHandler::loadVertexStage(const std::string& shaderName)
{
    std::string fullShaderName = shaderName + m_pDevice->getShaderPostfixAndExtension(SHADER_TYPE::VERTEX_SHADER);
    auto shaderItr = m_VertexStageCache.find(fullShaderName);
    if (shaderItr != m_VertexStageCache.end()) {
        std::weak_ptr<VertexStage>& shaderPtr = shaderItr->second;

        if (shaderPtr.expired()) {
            // The shader used to exist but has been deleted
            m_VertexStageCache.erase(shaderItr);
        } else {
            return shaderPtr.lock();
        }
    }

    // The vertex stage is not in the cache, compile it with the corresponding input layout info
    auto inputLayoutItr = m_InputLayoutInfos.find(shaderName);
    if (inputLayoutItr == m_InputLayoutInfos.end()) {
        LOG_WARNING("Could not find input layout info for shader: %s", shaderName.c_str());
        return nullptr;
    }

    InputLayout* pInputLayout = nullptr;
    Shader* pShader = m_pDevice->createShader(SHADER_TYPE::VERTEX_SHADER, shaderName, &inputLayoutItr->second, &pInputLayout);

    std::shared_ptr<VertexStage> vertexStage(new VertexStage(pInputLayout, pShader));
    m_VertexStageCache[shaderName] = vertexStage;

    return vertexStage;
}

std::shared_ptr<Shader> ShaderHandler::loadShader(const std::string& shaderName, SHADER_TYPE shaderType)
{
    if (HAS_FLAG(shaderType, SHADER_TYPE::VERTEX_SHADER)) {
        LOG_WARNING("Vertex shaders must be loaded using loadVertexStage, not loadShader: %s", shaderName.c_str());
        return nullptr;
    }

    std::string fullShaderName = shaderName + m_pDevice->getShaderPostfixAndExtension(shaderType);
    auto shaderItr = m_ShaderCache.find(fullShaderName);
    if (shaderItr != m_ShaderCache.end()) {
        std::weak_ptr<Shader>& shaderPtr = shaderItr->second;

        if (shaderPtr.expired()) {
            // The shader used to exist but has been deleted
            m_ShaderCache.erase(shaderItr);
        } else {
            return shaderPtr.lock();
        }
    }

    // The shader is not in the cache, compile it
    std::shared_ptr<Shader> shader(m_pDevice->createShader(shaderType, shaderName));
    m_ShaderCache[shaderName] = shader;

    return shader;
}
