#pragma once

#include <Engine/Rendering/APIAbstractions/DescriptorPool.hpp>

#include <vector>

class Device;
class IDescriptorSetLayout;

class DescriptorPoolHandler
{
public:
    DescriptorPoolHandler() = default;
    ~DescriptorPoolHandler();

    // Specify the settingsfor pools that will be created. Optimally, only one pool will be created, but more can be created if needed.
    void init(const DescriptorPoolInfo& poolInfos, Device* pDevice);

    DescriptorSet* allocateDescriptorSet(const IDescriptorSetLayout* pLayout, Device* pDevice);

private:
    std::vector<DescriptorPool*> m_DescriptorPools;

    DescriptorPoolInfo m_PoolInfos;
};
