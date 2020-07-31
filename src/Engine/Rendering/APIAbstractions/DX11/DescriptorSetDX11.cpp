#include "DescriptorSetDX11.hpp"

#include <Engine/Rendering/APIAbstractions/DX11/DescriptorSetLayoutDX11.hpp>
#include <Engine/Rendering/APIAbstractions/DX11/BufferDX11.hpp>
#include <Engine/Rendering/APIAbstractions/DX11/SamplerDX11.hpp>
#include <Engine/Rendering/APIAbstractions/DX11/TextureDX11.hpp>
#include <Engine/Utils/Rendering.hpp>

DescriptorSetDX11::DescriptorSetDX11(const DescriptorSetLayoutDX11* pDescriptorSetLayout, DescriptorPool* pDescriptorPool)
    :DescriptorSet(pDescriptorPool, pDescriptorSetLayout)
{}

void DescriptorSetDX11::updateUniformBufferDescriptor(SHADER_BINDING binding, IBuffer* pBuffer)
{
    BufferDX11* pBufferDX = reinterpret_cast<BufferDX11*>(pBuffer);
    const DescriptorSetLayoutDX11* pDescriptorSetLayoutDX = reinterpret_cast<const DescriptorSetLayoutDX11*>(m_pLayout);

    Binding<ID3D11Buffer*> bufferBinding = {
        .Resource       = pBufferDX->getBuffer(),
        .Binding        = (UINT)binding,
        .ShaderStages   = pDescriptorSetLayoutDX->getBindingShaderStages((uint32_t)binding)
    };

    m_BufferBindings.push_back(bufferBinding);
}

void DescriptorSetDX11::updateCombinedTextureSamplerDescriptor(SHADER_BINDING binding, Texture* pTexture, ISampler* pSampler)
{
    TextureDX11* pTextureDX = reinterpret_cast<TextureDX11*>(pTexture);
    SamplerDX11* pSamplerDX = reinterpret_cast<SamplerDX11*>(pSampler);
    const DescriptorSetLayoutDX11* pDescriptorSetLayoutDX = reinterpret_cast<const DescriptorSetLayoutDX11*>(m_pLayout);

    Binding<std::pair<ID3D11ShaderResourceView*, ID3D11SamplerState*>> combinedTextureSamplerBinding = {
        .Resource       = { pTextureDX->getSRV(), pSamplerDX->getSamplerState() },
        .Binding        = (UINT)binding,
        .ShaderStages   = pDescriptorSetLayoutDX->getBindingShaderStages((uint32_t)binding)
    };

    m_CombinedTextureSamplerBindings.push_back(combinedTextureSamplerBinding);
}

void DescriptorSetDX11::bind(ID3D11DeviceContext* pContext)
{
    for (const Binding<ID3D11Buffer*>& bufferBinding : m_BufferBindings) {
        ACTION_PER_CONTAINED_SHADER(bufferBinding.ShaderStages,
            pContext->VSSetConstantBuffers(bufferBinding.Binding, 1, &bufferBinding.Resource),
            pContext->HSSetConstantBuffers(bufferBinding.Binding, 1, &bufferBinding.Resource),
            pContext->DSSetConstantBuffers(bufferBinding.Binding, 1, &bufferBinding.Resource),
            pContext->GSSetConstantBuffers(bufferBinding.Binding, 1, &bufferBinding.Resource),
            pContext->PSSetConstantBuffers(bufferBinding.Binding, 1, &bufferBinding.Resource)
        )
    }

    for (const Binding<std::pair<ID3D11ShaderResourceView*, ID3D11SamplerState*>>& combinedTextureSamplerBinding : m_CombinedTextureSamplerBindings) {
        if (HAS_FLAG(combinedTextureSamplerBinding.ShaderStages, SHADER_TYPE::VERTEX_SHADER)) {
            pContext->VSSetShaderResources(combinedTextureSamplerBinding.Binding, 1, &combinedTextureSamplerBinding.Resource.first);
            pContext->VSSetSamplers(combinedTextureSamplerBinding.Binding, 1, &combinedTextureSamplerBinding.Resource.second);
        }

        if (HAS_FLAG(combinedTextureSamplerBinding.ShaderStages, SHADER_TYPE::HULL_SHADER)) {
            pContext->HSSetShaderResources(combinedTextureSamplerBinding.Binding, 1, &combinedTextureSamplerBinding.Resource.first);
            pContext->HSSetSamplers(combinedTextureSamplerBinding.Binding, 1, &combinedTextureSamplerBinding.Resource.second);
        }

        if (HAS_FLAG(combinedTextureSamplerBinding.ShaderStages, SHADER_TYPE::DOMAIN_SHADER)) {
            pContext->DSSetShaderResources(combinedTextureSamplerBinding.Binding, 1, &combinedTextureSamplerBinding.Resource.first);
            pContext->DSSetSamplers(combinedTextureSamplerBinding.Binding, 1, &combinedTextureSamplerBinding.Resource.second);
        }

        if (HAS_FLAG(combinedTextureSamplerBinding.ShaderStages, SHADER_TYPE::GEOMETRY_SHADER)) {
            pContext->GSSetShaderResources(combinedTextureSamplerBinding.Binding, 1, &combinedTextureSamplerBinding.Resource.first);
            pContext->GSSetSamplers(combinedTextureSamplerBinding.Binding, 1, &combinedTextureSamplerBinding.Resource.second);
        }

        if (HAS_FLAG(combinedTextureSamplerBinding.ShaderStages, SHADER_TYPE::FRAGMENT_SHADER)) {
            pContext->PSSetShaderResources(combinedTextureSamplerBinding.Binding, 1, &combinedTextureSamplerBinding.Resource.first);
            pContext->PSSetSamplers(combinedTextureSamplerBinding.Binding, 1, &combinedTextureSamplerBinding.Resource.second);
        }
    }
}
