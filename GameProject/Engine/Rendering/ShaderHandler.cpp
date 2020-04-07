#include "ShaderHandler.hpp"

#include <Engine/Utils/DirectXUtils.hpp>
#include <Engine/Utils/Logger.hpp>
#include <d3dcompiler.h>

ShaderHandler::ShaderHandler(ID3D11Device* device, ECSCore* pECS)
    :ComponentHandler({}, pECS, std::type_index(typeid(ShaderHandler))),
    m_pDevice(device)
{
    ComponentHandlerRegistration handlerReg = {};
    handlerReg.pComponentHandler = this;

    registerHandler(handlerReg);
}

ShaderHandler::~ShaderHandler()
{
    // Delete all shaders
    for (Program program : m_Programs) {
        if (program.vertexShader)
            program.vertexShader->Release();
        if (program.pixelShader)
            program.pixelShader->Release();
        if (program.inputLayout)
            program.inputLayout->Release();
    }
}

bool ShaderHandler::init()
{
    /* Compile all shaders and associate them with program enum names */
    // Compile Basic program
    std::vector<D3D11_INPUT_ELEMENT_DESC> inputLayoutDesc = {
        {
            "POSITION",                     // Semantic name in shader
            0,                              // Semantic index (not used)
            DXGI_FORMAT_R32G32B32_FLOAT,    // Element format
            0,                              // Input slot
            0,                              // Byte offset to first element
            D3D11_INPUT_PER_VERTEX_DATA,    // Input classification
            0                               // Instance step rate
        },
        {
            "NORMAL",                       // Semantic name in shader
            0,                              // Semantic index (not used)
            DXGI_FORMAT_R32G32B32_FLOAT,    // Element format
            0,                              // Input slot
            12,                             // Byte offset to first element
            D3D11_INPUT_PER_VERTEX_DATA,    // Input classification
            0                               // Instance step rate
        },
        {
            "TEXCOORD",                     // Semantic name in shader
            0,                              // Semantic index (not used)
            DXGI_FORMAT_R32G32_FLOAT,       // Element format
            0,                              // Input slot
            24,                             // Byte offset to first element
            D3D11_INPUT_PER_VERTEX_DATA,    // Input classification
            0                               // Instance step rate
        },
    };
    UINT vertexSize = 32;
    m_Programs.push_back(compileProgram(L"Basic", {VERTEX_SHADER, PIXEL_SHADER}, inputLayoutDesc, vertexSize));

    // Compile UI program
    inputLayoutDesc = {
        {
            "POSITION",                     // Semantic name in shader
            0,                              // Semantic index (not used)
            DXGI_FORMAT_R32G32_FLOAT,       // Element format
            0,                              // Input slot
            0,                              // Byte offset to first element
            D3D11_INPUT_PER_VERTEX_DATA,    // Input classification
            0                               // Instance step rate
        },
        {
            "TEXCOORD",                     // Semantic name in shader
            0,                              // Semantic index (not used)
            DXGI_FORMAT_R32G32_FLOAT,       // Element format
            0,                              // Input slot
            8,                              // Byte offset to first element
            D3D11_INPUT_PER_VERTEX_DATA,    // Input classification
            0                               // Instance step rate
        },
    };
    vertexSize = 16;
    m_Programs.push_back(compileProgram(L"UI", {VERTEX_SHADER, PIXEL_SHADER}, inputLayoutDesc, vertexSize));

    return true;
}

Program* ShaderHandler::getProgram(PROGRAM program)
{
    return &m_Programs[program];
}

Program ShaderHandler::compileProgram(LPCWSTR programName, std::vector<SHADER_TYPE> shaderTypes, std::vector<D3D11_INPUT_ELEMENT_DESC>& inputLayoutDesc, UINT vertexSize)
{
    Program program = {vertexSize, nullptr, nullptr, nullptr, nullptr, nullptr};

    HRESULT hr;
    std::wstring filePath;
    ID3DBlob* compiledCode = nullptr;

    for (SHADER_TYPE shaderType : shaderTypes) {
        switch (shaderType) {
            case VERTEX_SHADER:
                filePath = std::wstring(SHADERS_PATH) + programName + VS_POSTFIX;
                compiledCode = compileShader(filePath.c_str(), VS_ENTRYPOINT, VS_TARGET);

                do {
                    hr = m_pDevice->CreateVertexShader((const void*)compiledCode->GetBufferPointer(), compiledCode->GetBufferSize(), nullptr, &program.vertexShader);

                    if (FAILED(hr)) {
                        if (program.vertexShader) {
                            program.vertexShader->Release();
                            program.vertexShader = nullptr;
                        }

                        LOG_ERROR("Failed to create vertex shader from compiled code: %S", filePath.c_str());
                        system("pause");
                    }
                } while (program.vertexShader == nullptr);
                hr = m_pDevice->CreateInputLayout(&inputLayoutDesc[0], (UINT)inputLayoutDesc.size(), compiledCode->GetBufferPointer(),
                    compiledCode->GetBufferSize(), &program.inputLayout);
                if (FAILED(hr))
                    LOG_ERROR("Failed to create mesh input layout: %s", hresultToString(hr).c_str());
                break;

            case PIXEL_SHADER:
                filePath = std::wstring(SHADERS_PATH) + programName + PS_POSTFIX;
                compiledCode = compileShader(filePath.c_str(), PS_ENTRYPOINT, PS_TARGET);

                do {
                    hr = m_pDevice->CreatePixelShader((const void*)compiledCode->GetBufferPointer(), compiledCode->GetBufferSize(), nullptr, &program.pixelShader);

                    if (FAILED(hr)) {
                        if (program.pixelShader) {
                            program.pixelShader->Release();
                            program.pixelShader = nullptr;
                        }

                        LOG_ERROR("Failed to create pixel shader from compiled code: %S", filePath.c_str());
                        system("pause");
                    }
                } while (program.pixelShader == nullptr);
                break;
        }
    }

    return program;
}

ID3DBlob* ShaderHandler::compileShader(LPCWSTR fileName, LPCSTR entryPoint, LPCSTR targetVer)
{
    UINT compileFlags = D3DCOMPILE_OPTIMIZATION_LEVEL3 | D3DCOMPILE_ENABLE_STRICTNESS | D3DCOMPILE_WARNINGS_ARE_ERRORS;

    #ifdef _DEBUG
        compileFlags |= D3DCOMPILE_DEBUG;
    #endif

    ID3DBlob* compiledCode = nullptr, *errorMsgs = nullptr;

    do {
        HRESULT hr = D3DCompileFromFile(fileName, nullptr, nullptr, entryPoint, targetVer, compileFlags, 0, &compiledCode, &errorMsgs);

        if (FAILED(hr)) {
            LOG_ERROR("Failed to compile [%S]", fileName);

            if (errorMsgs) {
                LOG_ERROR("%s", (char*)errorMsgs->GetBufferPointer());
                errorMsgs->Release();
            }

            if (compiledCode) {
                compiledCode->Release();
                compiledCode = nullptr;
            }

            LOG_INFO("Edit the shader code and press any key to reattempt a compilation");
			std::getchar();
        }
    } while (compiledCode == nullptr);

    return compiledCode;
}
