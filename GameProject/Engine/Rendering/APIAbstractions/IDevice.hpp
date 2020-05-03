#pragma once

#define NOMINMAX
#include <DirectXMath.h>

struct SwapChainInfo {
    uint32_t FrameRateLimit;
    uint32_t Multisamples;
    bool Windowed;
};

class IBuffer;
class ICommandList;
class Window;
struct BufferInfo;

class IDevice
{
public:
    virtual bool init(const SwapChainInfo& swapChainInfo, Window* pWindow) = 0;

    virtual void clearBackBuffer() = 0;
    virtual void presentBackBuffer() = 0;

    virtual ICommandList* createCommandList() = 0;

    virtual BufferDX11* createBuffer(const BufferInfo& bufferInfo) = 0;
    virtual IBuffer* createVertexBuffer(const void* pVertices, size_t vertexSize, size_t vertexCount) = 0;
    virtual IBuffer* createIndexBuffer(const unsigned* pIndices, size_t indexCount) = 0;
};
