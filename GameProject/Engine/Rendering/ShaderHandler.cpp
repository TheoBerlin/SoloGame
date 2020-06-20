#include "ShaderHandler.hpp"

#include <Engine/Rendering/APIAbstractions/Device.hpp>
#include <Engine/Rendering/APIAbstractions/InputLayout.hpp>
#include <Engine/Utils/Debug.hpp>
#include <Engine/Utils/ECSUtils.hpp>

ShaderHandler::ShaderHandler(Device* pDevice)
    :m_pDevice(pDevice)
{
    // Create and map all input layout infos
    InputLayoutInfo inputLayoutInfo = {};
    inputLayoutInfo.Binding     = 0u;
    inputLayoutInfo.InputRate   = VERTEX_INPUT_RATE::PER_VERTEX;
    inputLayoutInfo.VertexInputAttributes = {
        {
            "POSITION",
            RESOURCE_FORMAT::R32G32B32_FLOAT
        },
        {
            "NORMAL",
            RESOURCE_FORMAT::R32G32B32_FLOAT
        },
        {
            "TEXCOORD",
            RESOURCE_FORMAT::R32G32_FLOAT
        }
    };

    m_InputLayoutInfos["Mesh"] = inputLayoutInfo;

    // Compile UI program
    inputLayoutInfo.VertexInputAttributes = {
        {
            "POSITION",
            RESOURCE_FORMAT::R32G32_FLOAT
        },
        {
            "TEXCOORD",
            RESOURCE_FORMAT::R32G32_FLOAT
        }
    };

    m_InputLayoutInfos["UI"] = inputLayoutInfo;
}

std::shared_ptr<Shader> ShaderHandler::loadShader(const std::string& shaderName, SHADER_TYPE shaderType)
{
    InputLayoutInfo* pInputLayoutInfo = nullptr;

    if (HAS_FLAG(shaderType, SHADER_TYPE::VERTEX_SHADER)) {
        // The vertex stage is not in the cache, compile it with the corresponding input layout info
        auto inputLayoutItr = m_InputLayoutInfos.find(shaderName);
        if (inputLayoutItr == m_InputLayoutInfos.end()) {
            LOG_ERROR("Could not find input layout info for shader: %s", shaderName.c_str());
            return nullptr;
        }

        pInputLayoutInfo = &inputLayoutItr->second;
    }

    std::string fullShaderName = shaderName + Shader::getTypePostfix(shaderType) + m_pDevice->getShaderFileExtension();
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
    std::shared_ptr<Shader> shader(m_pDevice->createShader(shaderType, shaderName, pInputLayoutInfo));
    m_ShaderCache[fullShaderName] = shader;

    return shader;
}
