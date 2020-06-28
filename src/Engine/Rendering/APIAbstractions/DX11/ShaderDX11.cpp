#include "ShaderDX11.hpp"

#include <Engine/Utils/Debug.hpp>
#include <Engine/Utils/DirectXUtils.hpp>
#include <Engine/Utils/Logger.hpp>

#include <d3dcompiler.h>

ShaderDX11* ShaderDX11::createVertexShader(SHADER_TYPE shaderType, ID3DBlob* pCompiledCode, const std::string& filePath, ID3D11Device* pDevice, const InputLayoutInfo* pInputLayoutInfo)
{
    InputLayoutDX11 inputLayout = {};
    if (!createInputLayout(inputLayout, pInputLayoutInfo, pCompiledCode, pDevice)) {
        return nullptr;
    }

    ID3D11VertexShader* pVertexShader = nullptr;
    HRESULT hr = pDevice->CreateVertexShader(pCompiledCode->GetBufferPointer(), pCompiledCode->GetBufferSize(), nullptr, &pVertexShader);
    if (FAILED(hr)) {
        SAFERELEASE(pVertexShader)
        LOG_ERROR("Failed to vertex shader from compiled code: %S", filePath.c_str());
        return nullptr;
    }

    return DBG_NEW ShaderDX11(shaderType, pVertexShader, nullptr, nullptr, nullptr, nullptr, &inputLayout);
}

ShaderDX11* ShaderDX11::createHullShader(SHADER_TYPE shaderType, ID3DBlob* pCompiledCode, const std::string& filePath, ID3D11Device* pDevice)
{
    ID3D11HullShader* pHullShader = nullptr;
    HRESULT hr = pDevice->CreateHullShader(pCompiledCode->GetBufferPointer(), pCompiledCode->GetBufferSize(), nullptr, &pHullShader);
    if (FAILED(hr)) {
        SAFERELEASE(pHullShader)
        LOG_ERROR("Failed to hull shader from compiled code: %S", filePath.c_str());
        return nullptr;
    }

    return DBG_NEW ShaderDX11(shaderType, nullptr, pHullShader, nullptr, nullptr, nullptr);
}

ShaderDX11* ShaderDX11::createDomainShader(SHADER_TYPE shaderType, ID3DBlob* pCompiledCode, const std::string& filePath, ID3D11Device* pDevice)
{
    ID3D11DomainShader* pDomainShader = nullptr;
    HRESULT hr = pDevice->CreateDomainShader(pCompiledCode->GetBufferPointer(), pCompiledCode->GetBufferSize(), nullptr, &pDomainShader);
    if (FAILED(hr)) {
        SAFERELEASE(pDomainShader)
        LOG_ERROR("Failed to domain shader from compiled code: %S", filePath.c_str());
        return nullptr;
    }

    return DBG_NEW ShaderDX11(shaderType, nullptr, nullptr, pDomainShader, nullptr, nullptr);
}

ShaderDX11* ShaderDX11::createGeometryShader(SHADER_TYPE shaderType, ID3DBlob* pCompiledCode, const std::string& filePath, ID3D11Device* pDevice)
{
    ID3D11GeometryShader* pGeometryShader = nullptr;
    HRESULT hr = pDevice->CreateGeometryShader(pCompiledCode->GetBufferPointer(), pCompiledCode->GetBufferSize(), nullptr, &pGeometryShader);
    if (FAILED(hr)) {
        SAFERELEASE(pGeometryShader)
        LOG_ERROR("Failed to geometry shader from compiled code: %S", filePath.c_str());
        return nullptr;
    }

    return DBG_NEW ShaderDX11(shaderType, nullptr, nullptr, nullptr, pGeometryShader, nullptr);
}

ShaderDX11* ShaderDX11::createFragmentShader(SHADER_TYPE shaderType, ID3DBlob* pCompiledCode, const std::string& filePath, ID3D11Device* pDevice)
{
    ID3D11PixelShader* pFragmentShader = nullptr;
    HRESULT hr = pDevice->CreatePixelShader(pCompiledCode->GetBufferPointer(), pCompiledCode->GetBufferSize(), nullptr, &pFragmentShader);
    if (FAILED(hr)) {
        SAFERELEASE(pFragmentShader)
        LOG_ERROR("Failed to fragment shader from compiled code: %S", filePath.c_str());
        return nullptr;
    }

    return DBG_NEW ShaderDX11(shaderType, nullptr, nullptr, nullptr, nullptr, pFragmentShader);
}

std::string ShaderDX11::getFilePostfix(SHADER_TYPE shaderType)
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

std::string ShaderDX11::getTargetVersion(SHADER_TYPE shaderType)
{
    switch (shaderType) {
        case SHADER_TYPE::VERTEX_SHADER:
            return "vs_5_0";
        case SHADER_TYPE::HULL_SHADER:
            return "hs_5_0";
        case SHADER_TYPE::DOMAIN_SHADER:
            return "ds_5_0";
        case SHADER_TYPE::GEOMETRY_SHADER:
            return "gs_5_0";
        case SHADER_TYPE::FRAGMENT_SHADER:
            return "ps_5_0";
        default:
            LOG_ERROR("Erroneous shader type: %d", (int)shaderType);
            return "vs_5_0";
    }
}

ShaderDX11::ShaderDX11(SHADER_TYPE shaderType, ID3D11VertexShader* m_pVertexShader, ID3D11HullShader* m_pHullShader,
        ID3D11DomainShader* m_pDomainShader, ID3D11GeometryShader* m_pGeometryShader, ID3D11PixelShader* m_pFragmentShader, InputLayoutDX11* pInputLayout)
    :Shader(shaderType),
    m_pVertexShader(m_pVertexShader),
    m_pHullShader(m_pHullShader),
    m_pDomainShader(m_pDomainShader),
    m_pGeometryShader(m_pGeometryShader),
    m_pFragmentShader(m_pFragmentShader),
    m_InputLayout(pInputLayout ? *pInputLayout : InputLayoutDX11())
{}

ShaderDX11::~ShaderDX11()
{
    SAFERELEASE(m_pVertexShader)
    SAFERELEASE(m_pHullShader)
    SAFERELEASE(m_pDomainShader)
    SAFERELEASE(m_pGeometryShader)
    SAFERELEASE(m_pFragmentShader)
    SAFERELEASE(m_InputLayout.pInputLayout)
}

ID3DBlob* ShaderDX11::compileShader(LPCWSTR fileName, LPCSTR targetVer)
{
    UINT compileFlags = D3DCOMPILE_OPTIMIZATION_LEVEL3 | D3DCOMPILE_ENABLE_STRICTNESS | D3DCOMPILE_WARNINGS_ARE_ERRORS;

    #ifdef _DEBUG
        compileFlags |= D3DCOMPILE_DEBUG;
    #endif

    ID3DBlob* pCompiledCode = nullptr, *errorMsgs = nullptr;

    HRESULT hr = D3DCompileFromFile(fileName, nullptr, nullptr, "main", targetVer, compileFlags, 0, &pCompiledCode, &errorMsgs);
    if (FAILED(hr)) {
        if (errorMsgs) {
            LOG_ERROR("%s", (char*)errorMsgs->GetBufferPointer());
            errorMsgs->Release();
        }

        SAFERELEASE(pCompiledCode)
        pCompiledCode = nullptr;
    }

    return pCompiledCode;
}
