#pragma once

#include <Engine/Rendering/APIAbstractions/InputLayout.hpp>
#include <Engine/Rendering/APIAbstractions/Shader.hpp>

#include <memory>
#include <unordered_map>

class Device;

class ShaderHandler
{
public:
    ShaderHandler(Device* pDevice);
    ~ShaderHandler() = default;

    std::shared_ptr<Shader> loadShader(const std::string& shaderPath, SHADER_TYPE shaderType);

    const InputLayoutInfo& getInputLayoutInfo(const std::string& shaderName) const;

private:
    // Mapping of vertex shader names to their input layout infos
    std::unordered_map<std::string, InputLayoutInfo> m_InputLayoutInfos;
    std::unordered_map<std::string, std::weak_ptr<Shader>> m_ShaderCache;

    Device* m_pDevice;
};
