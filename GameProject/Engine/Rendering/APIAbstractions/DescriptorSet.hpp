#pragma once

#include <Engine/Rendering/ShaderBindings.hpp>

class DescriptorPool;
class IBuffer;
class IDescriptorSetLayout;
class ISampler;
class Texture;

class DescriptorSet
{
public:
    DescriptorSet(DescriptorPool* pDescriptorPool, const IDescriptorSetLayout* pLayout);
    virtual ~DescriptorSet();

    virtual void updateUniformBufferDescriptor(SHADER_BINDING binding, IBuffer* pBuffer) = 0;
    virtual void updateSampledTextureDescriptor(SHADER_BINDING binding, Texture* pTexture) = 0;
    virtual void updateSamplerDescriptor(SHADER_BINDING binding, ISampler* pSampler) = 0;

    const IDescriptorSetLayout* getLayout() const { return m_pLayout; }

protected:
    DescriptorPool* m_pDescriptorPool;
    const IDescriptorSetLayout* m_pLayout;
};
