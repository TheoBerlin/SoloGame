#pragma once

#include <glm/glm.hpp>

#include <vector>

class IRenderPass;
class Texture;

struct FramebufferInfo {
    IRenderPass* pRenderPass;
    std::vector<Texture*> Attachments;
    glm::uvec2 Dimensions;
};

class Framebuffer
{
public:
    Framebuffer(const glm::uvec2& dimensions) :m_Dimensions(dimensions) {};
    virtual ~Framebuffer() = 0 {};

    inline const glm::uvec2 getDimensions() const { return m_Dimensions; }

protected:
    glm::uvec2 m_Dimensions;
};
