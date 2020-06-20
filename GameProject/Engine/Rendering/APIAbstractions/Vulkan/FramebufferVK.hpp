#pragma once

#include <Engine/Rendering/APIAbstractions/Framebuffer.hpp>

#include <vulkan/vulkan.h>

class DeviceVK;

class FramebufferVK : public IFramebuffer
{
public:
    static FramebufferVK* create(const FramebufferInfo& framebufferInfo, DeviceVK* pDevice);

public:
    FramebufferVK(VkFramebuffer framebuffer, DeviceVK* pDevice);
    ~FramebufferVK();

private:
    VkFramebuffer m_Framebuffer;
    DeviceVK* m_pDevice;
};
