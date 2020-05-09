#include "ShaderHandler.hpp"

#include <Engine/Rendering/APIAbstractions/Device.hpp>
#include <Engine/Rendering/APIAbstractions/InputLayout.hpp>
#include <Engine/Utils/ECSUtils.hpp>

ShaderHandler::ShaderHandler(Device* pDevice, ECSCore* pECS)
    :ComponentHandler(pECS, TID(ShaderHandler)),
    m_pDevice(pDevice)
{
    ComponentHandlerRegistration handlerReg = {};
    handlerReg.pComponentHandler = this;

    registerHandler(handlerReg);
}

ShaderHandler::~ShaderHandler()
{
    // Delete all shaders
    for (Program program : m_Programs) {
        delete program.pVertexShader;
        delete program.pHullShader;
        delete program.pDomainShader;
        delete program.pGeometryShader;
        delete program.pFragmentShader;

        delete program.pInputLayout;
    }
}

bool ShaderHandler::initHandler()
{
    /* Compile all shaders and associate them with program enum names */
    // Compile mesh program
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

    m_Programs.push_back(createProgram("Mesh", SHADER_TYPE::VERTEX_SHADER | SHADER_TYPE::FRAGMENT_SHADER, &inputLayoutInfo));

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
    m_Programs.push_back(createProgram("UI", SHADER_TYPE::VERTEX_SHADER | SHADER_TYPE::FRAGMENT_SHADER, &inputLayoutInfo));

    return true;
}

Program* ShaderHandler::getProgram(PROGRAM program)
{
    return &m_Programs[program];
}

Program ShaderHandler::createProgram(const std::string& programName, SHADER_TYPE shaderTypes, const InputLayoutInfo* pInputLayoutInfo)
{
    Program program = {};

    if (HAS_FLAG(shaderTypes, SHADER_TYPE::VERTEX_SHADER)) {
        program.pVertexShader = m_pDevice->createShader(SHADER_TYPE::VERTEX_SHADER, programName, pInputLayoutInfo, &program.pInputLayout);
    }

    if (HAS_FLAG(shaderTypes, SHADER_TYPE::HULL_SHADER)) {
        program.pHullShader = m_pDevice->createShader(SHADER_TYPE::HULL_SHADER, programName);
    }

    if (HAS_FLAG(shaderTypes, SHADER_TYPE::DOMAIN_SHADER)) {
        program.pDomainShader = m_pDevice->createShader(SHADER_TYPE::DOMAIN_SHADER, programName);
    }

    if (HAS_FLAG(shaderTypes, SHADER_TYPE::GEOMETRY_SHADER)) {
        program.pGeometryShader = m_pDevice->createShader(SHADER_TYPE::GEOMETRY_SHADER, programName);
    }

    if (HAS_FLAG(shaderTypes, SHADER_TYPE::FRAGMENT_SHADER)) {
        program.pFragmentShader = m_pDevice->createShader(SHADER_TYPE::FRAGMENT_SHADER, programName);
    }

    return program;
}
