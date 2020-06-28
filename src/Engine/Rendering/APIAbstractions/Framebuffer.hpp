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

class IFramebuffer
{
public:
    IFramebuffer() = default;
    virtual ~IFramebuffer() = 0 {};
};
