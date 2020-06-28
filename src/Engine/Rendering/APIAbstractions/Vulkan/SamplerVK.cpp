#include "SamplerVK.hpp"

#include <Engine/Rendering/APIAbstractions/Vulkan/DeviceVK.hpp>
#include <Engine/Rendering/APIAbstractions/Vulkan/GeneralResourcesVK.hpp>

SamplerVK* SamplerVK::create(const SamplerInfo& samplerInfo, DeviceVK* pDevice)
{
    VkSamplerCreateInfo samplerCreateInfo = {};
    samplerCreateInfo.sType             = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
    samplerCreateInfo.magFilter         = convertFilter(samplerInfo.FilterMag);
    samplerCreateInfo.minFilter         = convertFilter(samplerInfo.FilterMag);
    samplerCreateInfo.mipmapMode        = convertMipMapFilter(samplerInfo.FilterMip);
    samplerCreateInfo.addressModeU      = convertAddressMode(samplerInfo.AddressModeU);
    samplerCreateInfo.addressModeV      = convertAddressMode(samplerInfo.AddressModeV);
    samplerCreateInfo.addressModeW      = convertAddressMode(samplerInfo.AddressModeW);
    samplerCreateInfo.mipLodBias        = samplerInfo.MipLODBias;
    samplerCreateInfo.anisotropyEnable  = samplerInfo.AnisotropyEnabled;
    samplerCreateInfo.maxAnisotropy     = samplerInfo.MaxAnisotropy;
    samplerCreateInfo.compareEnable     = samplerInfo.CompareEnabled;
    if (samplerInfo.CompareEnabled) {
        samplerCreateInfo.compareOp     = convertCompareOp(samplerInfo.ComparisonFunc);
    }
    samplerCreateInfo.minLod            = samplerInfo.MinLOD;
    samplerCreateInfo.maxLod            = samplerInfo.MaxLOD;
    if (samplerCreateInfo.addressModeU == VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_BORDER ||
    samplerCreateInfo.addressModeV == VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_BORDER ||
    samplerCreateInfo.addressModeW == VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_BORDER){
        samplerCreateInfo.borderColor = convertBorderColor(samplerInfo.BorderColor);
    }

    VkSampler sampler = VK_NULL_HANDLE;
    if (vkCreateSampler(pDevice->getDevice(), &samplerCreateInfo, nullptr, &sampler) != VK_SUCCESS) {
        LOG_ERROR("Failed to create sampler");
        return nullptr;
    }

    return DBG_NEW SamplerVK(sampler, pDevice);
}

SamplerVK::SamplerVK(VkSampler sampler, DeviceVK* pDevice)
    :m_Sampler(sampler),
    m_pDevice(pDevice)
{}

SamplerVK::~SamplerVK()
{
    vkDestroySampler(m_pDevice->getDevice(), m_Sampler, nullptr);
}

VkFilter SamplerVK::convertFilter(FILTER filter)
{
    return filter == FILTER::NEAREST ? VK_FILTER_NEAREST : VK_FILTER_LINEAR;
}

VkSamplerMipmapMode SamplerVK::convertMipMapFilter(FILTER filter)
{
    return filter == FILTER::NEAREST ? VK_SAMPLER_MIPMAP_MODE_NEAREST : VK_SAMPLER_MIPMAP_MODE_LINEAR;
}

VkSamplerAddressMode SamplerVK::convertAddressMode(ADDRESS_MODE addressMode)
{
    switch (addressMode) {
        case ADDRESS_MODE::REPEAT:
            return VK_SAMPLER_ADDRESS_MODE_REPEAT;
        case ADDRESS_MODE::MIRROR_REPEAT:
            return VK_SAMPLER_ADDRESS_MODE_MIRRORED_REPEAT;
        case ADDRESS_MODE::MIRROR_CLAMP_TO_EDGE:
            return VK_SAMPLER_ADDRESS_MODE_MIRROR_CLAMP_TO_EDGE;
        case ADDRESS_MODE::CLAMP_TO_EDGE:
            return VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
        case ADDRESS_MODE::CLAMP_TO_BORDER:
            return VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_BORDER;
        default:
            LOG_WARNING("Erroneous address mode: %d", (uint32_t)addressMode);
            return VK_SAMPLER_ADDRESS_MODE_REPEAT;
    }
}

VkBorderColor SamplerVK::convertBorderColor(BORDER_COLOR borderColor)
{
    switch (borderColor) {
        case BORDER_COLOR::WHITE_OPAQUE:
            return VK_BORDER_COLOR_FLOAT_OPAQUE_WHITE;
        case BORDER_COLOR::BLACK_OPAQUE:
            return VK_BORDER_COLOR_FLOAT_OPAQUE_BLACK;
        case BORDER_COLOR::BLACK_TRANSPARENT:
            return VK_BORDER_COLOR_FLOAT_TRANSPARENT_BLACK;
        default:
            LOG_WARNING("Erroneous border color: %d", (uint32_t)borderColor);
            return VK_BORDER_COLOR_FLOAT_OPAQUE_BLACK;
    }
}
