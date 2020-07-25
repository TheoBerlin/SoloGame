#include "DescriptorPoolDX11.hpp"

#include <Engine/Rendering/APIAbstractions/DX11/DescriptorSetLayoutDX11.hpp>
#include <Engine/Utils/Debug.hpp>

DescriptorPoolDX11::DescriptorPoolDX11(const DescriptorPoolInfo& poolInfo)
    :DescriptorPool(poolInfo)
{}

DescriptorSetDX11* DescriptorPoolDX11::dAllocateDescriptorSet(const IDescriptorSetLayout* pDescriptorSetLayout)
{
    const DescriptorSetLayoutDX11* pDescriptorSetLayoutDX = reinterpret_cast<const DescriptorSetLayoutDX11*>(pDescriptorSetLayout);
    return DBG_NEW DescriptorSetDX11(pDescriptorSetLayoutDX, this);
}
