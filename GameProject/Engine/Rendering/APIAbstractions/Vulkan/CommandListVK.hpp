#pragma once

#include <vulkan/vulkan.h>

class DeviceVK;

class CommandListVK : public ICommandList
{
public:
    CommandListVK(VkCommandBuffer commandBuffer, VkCommandPool commandPool, DeviceVK* pDevice);
    ~CommandListVK();

    void execute() {};

    void beginRenderPass(IRenderPass* pRenderPass, const RenderPassBeginInfo& beginInfo) {};
    void bindPipeline(IPipeline* pPipeline) {};

    // Shader resources
    void bindDescriptorSet(DescriptorSet* pDescriptorSet) {};

    void map(IBuffer* pBuffer, void** ppMappedMemory) {};
    void unmap(IBuffer* pBuffer) {};

    void bindVertexBuffer(uint32_t firstBinding, IBuffer* pBuffer) {};
    void bindIndexBuffer(IBuffer* pBuffer) {};

    // Rasterizer
    void bindViewport(const Viewport* pViewport) {};

    void draw(size_t vertexCount) {};
    void drawIndexed(size_t indexCount) {};

    void convertTextureLayout(TEXTURE_LAYOUT oldLayout, TEXTURE_LAYOUT newLayout, Texture* pTexture) {};

    void copyBuffer(IBuffer* pSrc, IBuffer* pDst, size_t byteSize) {};

private:
    VkCommandBuffer m_CommandBuffer;
    // The command pool that created the command buffer
    VkCommandPool m_CommandPool;
    DeviceVK* m_pDevice;
};
