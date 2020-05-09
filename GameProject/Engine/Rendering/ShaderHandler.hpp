#pragma once

#include <Engine/ECS/ComponentHandler.hpp>
#include <Engine/Rendering/APIAbstractions/InputLayout.hpp>
#include <Engine/Rendering/APIAbstractions/Shader.hpp>

#include <vector>

enum PROGRAM {
    MESH = 0,
    UI = 1
};

struct Program {
    InputLayout* pInputLayout;
    Shader* pVertexShader;
    Shader* pHullShader;
    Shader* pDomainShader;
    Shader* pGeometryShader;
    Shader* pFragmentShader;
};

class Device;

class ShaderHandler : public ComponentHandler
{
public:
    ShaderHandler(Device* pDevice, ECSCore* pECS);
    ~ShaderHandler();

    virtual bool initHandler() override;

    Program* getProgram(PROGRAM program);

private:
    Program createProgram(const std::string& programName, SHADER_TYPE shaderTypes, const InputLayoutInfo* inputLayoutInfo);

    std::vector<Program> m_Programs;

    Device* m_pDevice;
};
