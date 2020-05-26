#pragma once

#include <Engine/Rendering/APIAbstractions/InputLayout.hpp>
#include <Engine/Rendering/APIAbstractions/Shader.hpp>

#include <memory>
#include <unordered_map>

class VertexStage
{
public:
    VertexStage(InputLayout* pInputLayout, Shader* pVertexShader)
        :m_pInputLayout(pInputLayout),
        m_pVertexShader(pVertexShader)
    {}

    ~VertexStage()
    {
        delete m_pInputLayout;
        delete m_pVertexShader;
    }

    InputLayout* getInputLayout() const { return m_pInputLayout; }
    Shader* getVertexShader() const { return m_pVertexShader; }

private:
    InputLayout* m_pInputLayout;
    Shader* m_pVertexShader;
};

class Device;

class ShaderHandler
{
public:
    ShaderHandler(Device* pDevice);
    ~ShaderHandler() = default;

    // The shader path must not include the path to the base Shaders folder
    std::shared_ptr<VertexStage> loadVertexStage(const std::string& shaderPath);
    std::shared_ptr<Shader> loadShader(const std::string& shaderPath, SHADER_TYPE shaderType);

private:
    // Mapping of vertex shader names to their input layout infos
    std::unordered_map<std::string, InputLayoutInfo> m_InputLayoutInfos;
    std::unordered_map<std::string, std::weak_ptr<VertexStage>> m_VertexStageCache;
    // Stores all shader types except vertex shaders, they are stored in the above hash map
    std::unordered_map<std::string, std::weak_ptr<Shader>> m_ShaderCache;

    Device* m_pDevice;
};
