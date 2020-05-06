#pragma once

#include <Engine/Rendering/APIAbstractions/Shader.hpp>
#include <Engine/Rendering/APIAbstractions/Texture.hpp>

class IBuffer;
struct Program;

class ICommandList
{
public:
    virtual ~ICommandList() = 0 {};

    virtual void execute() = 0;

    virtual void map(IBuffer* pBuffer, void** ppMappedMemory) = 0;
    virtual void unmap(IBuffer* pBuffer) = 0;
    virtual void bindBuffer(int slot, SHADER_TYPE shaderStages, IBuffer* pBuffer) = 0;
    virtual void bindVertexBuffer(int slot, size_t vertexSize, IBuffer* pBuffer) = 0;
    virtual void bindIndexBuffer(IBuffer* pBuffer) = 0;

    virtual void bindShaderResourceTexture(int slot, SHADER_TYPE shaderStages, Texture* pTexture) = 0;
    virtual void bindRenderTarget(Texture* pRenderTarget, Texture* pDepthStencil) = 0;

    virtual void bindShaders(const Program* program) = 0;

    virtual void draw(size_t vertexCount) = 0;
    virtual void drawIndexed(size_t indexCount) = 0;

    virtual void convertTextureLayout(TEXTURE_LAYOUT oldLayout, TEXTURE_LAYOUT newLayout, Texture* pTexture) = 0;
};
