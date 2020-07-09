#pragma once

#include <Engine/Rendering/APIAbstractions/Framebuffer.hpp>

#include <vulkan/vulkan.h>

class DeviceVK;

class FramebufferVK : public Framebuffer
{
public:
    static FramebufferVK* create(const FramebufferInfo& framebufferInfo, DeviceVK* pDevice);

public:
    FramebufferVK(VkFramebuffer framebuffer, const glm::uvec2& dimensions, DeviceVK* pDevice);
    ~FramebufferVK();

    inline VkFramebuffer getFramebuffer() { return m_Framebuffer; }

private:
    VkFramebuffer m_Framebuffer;
    DeviceVK* m_pDevice;
};
