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
    Resource Resource;
    UINT Binding;
    SHADER_TYPE ShaderStages;
};

class DescriptorSetDX11 : public DescriptorSet
{
public:
    DescriptorSetDX11(const DescriptorSetLayoutDX11* pDescriptorSetLayout, DescriptorPool* pDescriptorPool);
    ~DescriptorSetDX11() = default;

    void updateUniformBufferDescriptor(SHADER_BINDING binding, IBuffer* pBuffer) override final;
    void updateCombinedTextureSamplerDescriptor(SHADER_BINDING binding, Texture* pTexture, ISampler* pSampler) override final;

    void bind(ID3D11DeviceContext* pContext);

private:
    std::vector<Binding<ID3D11Buffer*>> m_BufferBindings;
    std::vector<Binding<std::pair<ID3D11ShaderResourceView*, ID3D11SamplerState*>>> m_CombinedTextureSamplerBindings;

    // Contains information on which shaders the bindings belong to
    const DescriptorSetLayoutDX11* m_pLayout;
};
