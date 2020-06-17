#pragma once

#include <Engine/Rendering/APIAbstractions/ISampler.hpp>

#include <vulkan/vulkan.h>

class DeviceVK;

class SamplerVK : public ISampler
{
public:
    static SamplerVK* create(const SamplerInfo& samplerInfo, DeviceVK* pDevice);

public:
    SamplerVK(VkSampler sampler, DeviceVK* pDevice);
    ~SamplerVK();

    VkSampler getSampler() { return m_Sampler; }

private:
    static VkFilter convertFilter(FILTER filter);
    static VkSamplerMipmapMode convertMipMapFilter(FILTER filter);
    static VkSamplerAddressMode convertAddressMode(ADDRESS_MODE addressMode);
    static VkCompareOp convertCompareOp(COMPARISON_FUNC comparisonFunc);
    static VkBorderColor convertBorderColor(BORDER_COLOR borderColor);

private:
    VkSampler m_Sampler;
    DeviceVK* m_pDevice;
};
