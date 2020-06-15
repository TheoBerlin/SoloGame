#pragma once

#include <Engine/Rendering/APIAbstractions/Shader.hpp>

#define NOMINMAX
#include <d3d11.h>
#include <string>

class ShaderDX11 : public Shader
{
public:
    static ID3DBlob* compileShader(LPCWSTR fileName, LPCSTR targetVer);

    static ShaderDX11* createVertexShader(SHADER_TYPE shaderType, ID3DBlob* pCompiledCode, const std::string& filePath, ID3D11Device* pDevice);
    static ShaderDX11* createHullShader(SHADER_TYPE shaderType, ID3DBlob* pCompiledCode, const std::string& filePath, ID3D11Device* pDevice);
    static ShaderDX11* createDomainShader(SHADER_TYPE shaderType, ID3DBlob* pCompiledCode, const std::string& filePath, ID3D11Device* pDevice);
    static ShaderDX11* createGeometryShader(SHADER_TYPE shaderType, ID3DBlob* pCompiledCode, const std::string& filePath, ID3D11Device* pDevice);
    static ShaderDX11* createFragmentShader(SHADER_TYPE shaderType, ID3DBlob* pCompiledCode, const std::string& filePath, ID3D11Device* pDevice);

    static std::string getFilePostfix(SHADER_TYPE shaderType);
    static std::string getTargetVersion(SHADER_TYPE shaderType);

public:
    ShaderDX11(SHADER_TYPE shaderType, ID3D11VertexShader* m_pVertexShader, ID3D11HullShader* m_pHullShader,
        ID3D11DomainShader* m_pDomainShader, ID3D11GeometryShader* m_pGeometryShader, ID3D11PixelShader* m_pFragmentShader);
    ~ShaderDX11();

    inline ID3D11VertexShader* getVertexShader()        { return m_pVertexShader; }
    inline ID3D11HullShader* getHullShader()            { return m_pHullShader; }
    inline ID3D11DomainShader* getDomainShader()        { return m_pDomainShader; }
    inline ID3D11GeometryShader* getGeometryShader()    { return m_pGeometryShader; }
    inline ID3D11PixelShader* getFragmentShader()       { return m_pFragmentShader; }

private:
    ID3D11VertexShader*     m_pVertexShader;
    ID3D11HullShader*       m_pHullShader;
    ID3D11DomainShader*     m_pDomainShader;
    ID3D11GeometryShader*   m_pGeometryShader;
    ID3D11PixelShader*      m_pFragmentShader;
};
