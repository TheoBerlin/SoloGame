#pragma once

#include <vulkan/vulkan.h>

class DeviceVK;

class CommandListVK : public ICommandList
{
public:
    CommandListVK(VkCommandBuffer commandBuffer, VkCommandPool commandPool, DeviceVK* pDevice);
    ~CommandListVK() = default;

    bool begin(COMMAND_LIST_USAGE usageFlags, CommandListBeginInfo* pBeginInfo) override final;
    bool reset() override final;
    bool end() override final { return vkEndCommandBuffer(m_CommandBuffer) == VK_SUCCESS; }

    void executeSecondaryCommandList(ICommandList* pSecondaryCommandList) override final;

    void beginRenderPass(IRenderPass* pRenderPass, const RenderPassBeginInfo& beginInfo) override final;
    void nextSubpass(COMMAND_LIST_LEVEL subpassCommandListLevel) override final;
    void endRenderPass() override final;
    void bindPipeline(IPipeline* pPipeline) override final;

    // Shader resources
    void bindDescriptorSet(DescriptorSet* pDescriptorSet, IPipelineLayout* pPipelineLayout, uint32_t setNr) override final;

    void bindVertexBuffer(uint32_t firstBinding, IBuffer* pBuffer) override final;
    void bindIndexBuffer(IBuffer* pBuffer) override final;

    // Rasterizer
    void bindViewport(const Viewport* pViewport) override final;
    void bindScissor(const Rectangle2D& scissorRectangle) override final;

    void draw(size_t vertexCount) override final;
    void drawIndexed(size_t indexCount) override final;

    void convertTextureLayout(TEXTURE_LAYOUT oldLayout, TEXTURE_LAYOUT newLayout, Texture* pTexture, PIPELINE_STAGE srcStage, PIPELINE_STAGE dstStage) override final;

    void copyBuffer(IBuffer* pSrc, IBuffer* pDst, size_t byteSize) override final;
    void copyBufferToTexture(IBuffer* pBuffer, Texture* pTexture, uint32_t width, uint32_t height) override final;

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
