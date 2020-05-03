#pragma once

#include <Engine/Rendering/APIAbstractions/Shader.hpp>

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

    virtual void bindShaders(const Program* program) = 0;

    virtual void draw(size_t vertexCount) = 0;
    virtual void drawIndexed(size_t indexCount) = 0;
};
