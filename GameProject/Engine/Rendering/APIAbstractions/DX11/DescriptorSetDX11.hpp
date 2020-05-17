#pragma once

#include <Engine/Rendering/APIAbstractions/DescriptorSet.hpp>
#include <Engine/Rendering/APIAbstractions/Shader.hpp>

#define NOMINMAX
#include <d3d11.h>
#include <vector>

class BufferDX11;
class DescriptorSetLayoutDX11;
class TextureDX11;

template <typename Resource>
struct Binding {
    Resource* pResource;
    UINT Binding;
    SHADER_TYPE ShaderStages;
};

class DescriptorSetDX11 : public DescriptorSet
{
public:
    DescriptorSetDX11(const DescriptorSetLayoutDX11* pDescriptorSetLayout, DescriptorPool* pDescriptorPool);
    ~DescriptorSetDX11() = default;

    void writeUniformBufferDescriptor(SHADER_BINDING binding, IBuffer* pBuffer) override final;
    void writeSampledTextureDescriptor(SHADER_BINDING binding, Texture* pTexture) override final;
    void writeSamplerDescriptor(SHADER_BINDING binding, ISampler* pSampler) override final;

    void bind(ID3D11DeviceContext* pContext);

private:
    std::vector<Binding<ID3D11Buffer>> m_BufferBindings;
    std::vector<Binding<ID3D11ShaderResourceView>> m_SampledTextureBindings;
    std::vector<Binding<ID3D11SamplerState>> m_SamplerBindings;

    // Contains information on which shaders the bindings belong to
    const DescriptorSetLayoutDX11* m_pLayout;
};
