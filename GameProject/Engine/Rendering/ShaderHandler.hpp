#pragma once

#include <Engine/ECS/ComponentHandler.hpp>
#include <d3d11.h>
#include <vector>

const LPCWSTR SHADERS_PATH = L"Engine/Rendering/Shaders/";

const LPCSTR VS_ENTRYPOINT = "VS_main";
const LPCSTR VS_TARGET = "vs_5_0";
const LPCWSTR VS_POSTFIX = L"_VS.hlsl";

const LPCSTR PS_ENTRYPOINT = "PS_main";
const LPCSTR PS_TARGET = "ps_5_0";
const LPCWSTR PS_POSTFIX = L"_PS.hlsl";

enum SHADER_TYPE {
    VERTEX_SHADER,
    HULL_SHADER,
    DOMAIN_SHADER,
    GEOMETRY_SHADER,
    PIXEL_SHADER
};

enum PROGRAM
{
    BASIC = 0
};

struct Program {
    UINT vertexSize;
    ID3D11InputLayout* inputLayout;
    ID3D11VertexShader* vertexShader;
    ID3D11HullShader* hullShader;
    ID3D11DomainShader* domainShader;
    ID3D11GeometryShader* geometryShader;
    ID3D11PixelShader* pixelShader;
};

class ShaderHandler : public ComponentHandler
{
public:
    ShaderHandler(ID3D11Device* device, SystemSubscriber* systemSubscriber);
    ~ShaderHandler();

    Program* getProgram(PROGRAM program);

    // Creates new input layout using description
    Program compileProgram(ID3D11Device* device, LPCWSTR programName, std::vector<SHADER_TYPE> shaderTypes,
        std::vector<D3D11_INPUT_ELEMENT_DESC>& inputLayoutDesc, UINT vertexSize);

    ID3DBlob* compileShader(LPCWSTR fileName, LPCSTR entryPoint, LPCSTR targetVer);

private:
    std::vector<Program> programs;
};
