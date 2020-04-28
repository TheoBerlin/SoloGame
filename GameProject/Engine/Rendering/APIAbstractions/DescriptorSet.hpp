#pragma once

class IDescriptorSet
{
public:
    virtual void writeUniformDescriptor(uint32_t binding, )
    void writeUniformBufferDescriptor(const BufferVK* pBuffer, uint32_t binding);

    virtual bool init() = 0;
};
