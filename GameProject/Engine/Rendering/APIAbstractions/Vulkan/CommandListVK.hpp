#pragma once

#include <vulkan/vulkan.h>

class DeviceVK;

class CommandListVK : public ICommandList
{
public:
    CommandListVK(VkCommandBuffer commandBuffer, VkCommandPool commandPool, DeviceVK* pDevice);
    ~CommandListVK();

    bool begin(COMMAND_LIST_USAGE usageFlags, CommandListBeginInfo* pBeginInfo) override final;
    bool reset() override final;
    inline bool end() override final { return vkEndCommandBuffer(m_CommandBuffer) == VK_SUCCESS; }

    void beginRenderPass(IRenderPass* pRenderPass, const RenderPassBeginInfo& beginInfo) override final {};
    void bindPipeline(IPipeline* pPipeline) override final {};

    // Shader resources
    void bindDescriptorSet(DescriptorSet* pDescriptorSet) override final {};

    void map(IBuffer* pBuffer, void** ppMappedMemory) override final {};
    void unmap(IBuffer* pBuffer) override final {};

    void bindVertexBuffer(uint32_t firstBinding, IBuffer* pBuffer) override final {};
    void bindIndexBuffer(IBuffer* pBuffer) override final {};

    // Rasterizer
    void bindViewport(const Viewport* pViewport) override final {};

    void draw(size_t vertexCount) override final {};
    void drawIndexed(size_t indexCount) override final {};

    void convertTextureLayout(TEXTURE_LAYOUT oldLayout, TEXTURE_LAYOUT newLayout, Texture* pTexture) override final {};

    void copyBuffer(IBuffer* pSrc, IBuffer* pDst, size_t byteSize) override final {};

    inline VkCommandBuffer getCommandBuffer() { return m_CommandBuffer; }

private:
    static VkCommandBufferUsageFlags convertUsageFlags(COMMAND_LIST_USAGE usageFlags);
    static VkCommandBufferInheritanceInfo convertBeginInfo(CommandListBeginInfo* pBeginInfo);

private:
    VkCommandBuffer m_CommandBuffer;
    // The command pool that created the command buffer
    VkCommandPool m_CommandPool;
    DeviceVK* m_pDevice;
};
