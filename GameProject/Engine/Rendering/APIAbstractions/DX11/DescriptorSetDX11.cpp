#include "DescriptorSetDX11.hpp"

#include <Engine/Rendering/APIAbstractions/DX11/DescriptorSetLayoutDX11.hpp>
#include <Engine/Rendering/APIAbstractions/DX11/BufferDX11.hpp>
#include <Engine/Rendering/APIAbstractions/DX11/SamplerDX11.hpp>
#include <Engine/Rendering/APIAbstractions/DX11/TextureDX11.hpp>
#include <Engine/Utils/Rendering.hpp>

DescriptorSetDX11::DescriptorSetDX11(const DescriptorSetLayoutDX11* pDescriptorSetLayout, DescriptorPool* pDescriptorPool)
    :DescriptorSet(pDescriptorPool, pDescriptorSetLayout),
    m_pLayout(pDescriptorSetLayout)
{}

void DescriptorSetDX11::writeUniformBufferDescriptor(SHADER_BINDING binding, IBuffer* pBuffer)
{
    BufferDX11* pBufferDX = reinterpret_cast<BufferDX11*>(pBuffer);
    Binding<ID3D11Buffer> bufferBinding = {
        .pResource      = pBufferDX->getBuffer(),
        .Binding        = (UINT)binding,
        .ShaderStages   = m_pLayout->getBindingShaderStages((uint32_t)binding)
    };

    m_BufferBindings.push_back(bufferBinding);
}

void DescriptorSetDX11::writeSampledTextureDescriptor(SHADER_BINDING binding, Texture* pTexture)
{
    TextureDX11* pTextureDX = reinterpret_cast<TextureDX11*>(pTexture);
    Binding<ID3D11ShaderResourceView> textureBinding = {
        .pResource      = pTextureDX->getSRV(),
        .Binding        = (UINT)binding,
        .ShaderStages   = m_pLayout->getBindingShaderStages((uint32_t)binding)
    };

    m_SampledTextureBindings.push_back(textureBinding);
}

void DescriptorSetDX11::writeSamplerDescriptor(SHADER_BINDING binding, ISampler* pSampler)
{
    SamplerDX11* pSamplerDX = reinterpret_cast<SamplerDX11*>(pSampler);
    Binding<ID3D11SamplerState> samplerBinding = {
        .pResource      = pSamplerDX->getSamplerState(),
        .Binding        = (UINT)binding,
        .ShaderStages   = m_pLayout->getBindingShaderStages((uint32_t)binding)
    };

    m_SamplerBindings.push_back(samplerBinding);
}

void DescriptorSetDX11::bind(ID3D11DeviceContext* pContext)
{
    for (const Binding<ID3D11Buffer>& bufferBinding : m_BufferBindings) {
        ACTION_PER_CONTAINED_SHADER(bufferBinding.ShaderStages,
            pContext->VSSetConstantBuffers(bufferBinding.Binding, 1, &bufferBinding.pResource),
            pContext->HSSetConstantBuffers(bufferBinding.Binding, 1, &bufferBinding.pResource),
            pContext->DSSetConstantBuffers(bufferBinding.Binding, 1, &bufferBinding.pResource),
            pContext->GSSetConstantBuffers(bufferBinding.Binding, 1, &bufferBinding.pResource),
            pContext->PSSetConstantBuffers(bufferBinding.Binding, 1, &bufferBinding.pResource)
        )
    }

    for (const Binding<ID3D11ShaderResourceView>& textureBinding : m_SampledTextureBindings) {
        ACTION_PER_CONTAINED_SHADER(textureBinding.ShaderStages,
            pContext->VSSetShaderResources(textureBinding.Binding, 1, &textureBinding.pResource),
            pContext->HSSetShaderResources(textureBinding.Binding, 1, &textureBinding.pResource),
            pContext->DSSetShaderResources(textureBinding.Binding, 1, &textureBinding.pResource),
            pContext->GSSetShaderResources(textureBinding.Binding, 1, &textureBinding.pResource),
            pContext->PSSetShaderResources(textureBinding.Binding, 1, &textureBinding.pResource)
        )
    }

    for (const Binding<ID3D11SamplerState>& samplerBinding : m_SamplerBindings) {
        ACTION_PER_CONTAINED_SHADER(samplerBinding.ShaderStages,
            pContext->VSSetSamplers(samplerBinding.Binding, 1, &samplerBinding.pResource),
            pContext->HSSetSamplers(samplerBinding.Binding, 1, &samplerBinding.pResource),
            pContext->DSSetSamplers(samplerBinding.Binding, 1, &samplerBinding.pResource),
            pContext->GSSetSamplers(samplerBinding.Binding, 1, &samplerBinding.pResource),
            pContext->PSSetSamplers(samplerBinding.Binding, 1, &samplerBinding.pResource)
        )
    }
}
