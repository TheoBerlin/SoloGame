#pragma once

#include <Engine/Rendering/APIAbstractions/PipelineLayout.hpp>

#include <vulkan/vulkan.h>

class DeviceVK;

class PipelineLayoutVK : public IPipelineLayout
{
public:
    static PipelineLayoutVK* create(std::vector<IDescriptorSetLayout*> descriptorSetLayouts, DeviceVK* pDevice);

public:
    PipelineLayoutVK(VkPipelineLayout pipelineLayout, DeviceVK* pDevice);
    ~PipelineLayoutVK();

    inline VkPipelineLayout getPipelineLayout() { return m_PipelineLayout; }

private:
    VkPipelineLayout m_PipelineLayout;
    DeviceVK* m_pDevice;
};
