#pragma once

#include <stdint.h>
#include <string>

class DescriptorCounts
{
public:
    DescriptorCounts();
    ~DescriptorCounts() = default;

    DescriptorCounts& operator+=(const DescriptorCounts& other);
    DescriptorCounts& operator-=(const DescriptorCounts& other);
    DescriptorCounts& operator*=(uint32_t factor);
    friend DescriptorCounts operator+(DescriptorCounts lhs, const DescriptorCounts& rhs)    { return lhs += rhs; }
    friend DescriptorCounts operator*(DescriptorCounts lhs, uint32_t factor)                { return lhs *= factor; }

    void setAll(uint32_t descriptorCount);
    // Set the descriptor counts to be at least as large as the other descriptor counts object
    void ceil(const DescriptorCounts& other);

    bool contains(const DescriptorCounts& other) const;

    std::string toString() const;

    // Gets the amount of unique descriptor types, i.e. the amount of descriptor types whose counts aren't 0
    uint32_t getDescriptorTypeCount() const;

    uint32_t m_UniformBuffers;
    uint32_t m_CombinedTextureSamplers;
};
