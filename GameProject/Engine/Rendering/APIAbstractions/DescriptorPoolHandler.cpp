#include "DescriptorPoolHandler.hpp"

#include <Engine/Rendering/APIAbstractions/Device.hpp>
#include <Engine/Rendering/APIAbstractions/DescriptorSetLayout.hpp>

#include <Engine/Utils/Logger.hpp>

DescriptorPoolHandler::~DescriptorPoolHandler()
{
    for (DescriptorPool* pDescriptorPool : m_DescriptorPools) {
        delete pDescriptorPool;
    }
}

void DescriptorPoolHandler::init(const DescriptorCounts& poolSizes, Device* pDevice)
{
    m_PoolSizes = poolSizes;

    m_DescriptorPools.push_back(pDevice->createDescriptorPool(poolSizes));
}

DescriptorSet* DescriptorPoolHandler::allocateDescriptorSet(const IDescriptorSetLayout* pLayout, Device* pDevice)
{
    DescriptorCounts descriptorsToAllocate = pLayout->getDescriptorCounts();
    DescriptorSet* pDescriptorSet = nullptr;

    for (DescriptorPool* pDescriptorPool : m_DescriptorPools) {
        if (pDescriptorPool->hasRoomFor(descriptorsToAllocate)) {
            pDescriptorSet = pDescriptorPool->allocateDescriptorSet(pLayout);

            if (pDescriptorSet) {
                return pDescriptorSet;
            }
        }
    }

    // No descriptor pool was able to allocate the descriptor set, create a new pool
    // The new pool needs to be at least large enough to fit the new descriptor set
    DescriptorCounts newPoolSize = m_PoolSizes;
    newPoolSize.ceil(descriptorsToAllocate);

    DescriptorCounts newRecommendedPoolSize = m_PoolSizes * (uint32_t)m_DescriptorPools.size() + descriptorsToAllocate;
    LOG_INFO("Pool size exceeded, new recommended pool size: %s", newRecommendedPoolSize.toString().c_str());

    m_DescriptorPools.push_back(pDevice->createDescriptorPool(newPoolSize));

    // Last attempt to allocate descriptor set
    pDescriptorSet = m_DescriptorPools.back()->allocateDescriptorSet(pLayout);
    if (!pDescriptorSet) {
        LOG_ERROR("Failed to allocate descriptor set");
    }

    return pDescriptorSet;
}
