#pragma once

#include <functional>
#include <mutex>

template <typename ResourceType>
class ResourcePool;

template <typename ResourceType>
class PooledResource
{
public:
    ~PooledResource() = default;

    ResourceType& operator*()   { return *m_pResource; }
    ResourceType* operator->()  { return m_pResource; }

    inline void release()
    {
        m_pResourcePool->release(m_PoolIndex);
    }

protected:
    friend ResourcePool<ResourceType>;
    PooledResource(ResourceType* pResource, size_t poolIdx, ResourcePool<ResourceType>* pResourcePool)
        :m_pResource(pResource),
        m_PoolIndex(poolIdx)
    {}

private:
    ResourceType* m_pResource;
    ResourcePool<ResourceType>* m_pResourcePool;

    size_t m_PoolIndex;
};

template <typename ResourceType>
class ResourcePool
{
public:
    ResourcePool() = default;

    ~ResourcePool()
    {
        if (m_Deleter) {
            for (ResourceType* pResource : m_Resources) {
                m_Deleter(pResource);
            }
        } else {
            for (ResourceType* pResource : m_Resources) {
                delete pResource;
            }
        }
    }

    void init(std::vector<ResourceType*> resources, std::function<void(ResourceType*)> m_Deleter = nullptr)
    {
        m_Resources = resources;

        m_FreeIndices.resize(resources.size());
        for (size_t freeIdx = 0u; freeIdx < m_FreeIndices.size(); freeIdx++) {
            m_FreeIndices[freeIdx] = freeIdx;
        }
    }

    PooledResource<ResourceType> acquire()
    {
        std::unique_lock<std::mutex> lock(m_VectorsLock);

        // If all resources are busy, wait until one is free
        m_AllBusyCondition.wait(lock, [this](){ return !m_FreeIndices.empty(); });

        PooledResource<ResourceType> resource(m_Resources[m_FreeIndices.back()], m_FreeIndices.back(), this);
        m_FreeIndices.pop_back();
        lock.unlock();

        return resource;
    }

protected:
    friend PooledResource<ResourceType*>;
    void release(size_t resourceIndex)
    {
        std::scoped_lock<std::mutex> lock(m_VectorsLock);

        m_FreeIndices.push_back(resourceIndex);
        if (m_FreeIndices.size() == 1) {
            m_AllBusyCondition.notify_one();
        }
    }

private:
    std::vector<ResourceType*> m_Resources;
    std::vector<size_t> m_FreeIndices;

    std::function<void(ResourceType*)> m_Deleter;

    std::mutex m_VectorsLock;
    // Used to make threads wait until one resource instance is free
    std::condition_variable m_AllBusyCondition;
};
