#pragma once

#include <Engine/Rendering/APIAbstractions/Pipeline.hpp>

#include <vulkan/vulkan.h>

class DeviceVK;

class PipelineVK : public IPipeline
{
public:
    static PipelineVK* create(const PipelineInfo& pipelineInfo, DeviceVK* pDevice);

public:
    PipelineVK(VkPipeline pipeline, std::vector<std::shared_ptr<Shader>>& shaders, DeviceVK* pDevice);
    ~PipelineVK();

    inline VkPipeline getPipeline() { return m_Pipeline; }

private:
    static VkPipelineShaderStageCreateInfo writeShaderStageInfo(const Shader* pShader);
    static VkPrimitiveTopology convertPrimitiveTopology(PRIMITIVE_TOPOLOGY primitiveTopology);

private:
    VkPipeline m_Pipeline;
    std::vector<std::shared_ptr<Shader>>& m_Shaders;

    DeviceVK* m_pDevice;
};
