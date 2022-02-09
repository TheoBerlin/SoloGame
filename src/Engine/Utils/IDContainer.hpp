#pragma once

#include <vector>

class IDContainer
{
public:
    virtual bool HasElement(uint32_t ID) const = 0;

    virtual uint32_t Size() const = 0;
    virtual const std::vector<uint32_t>& GetIDs() const = 0;
    virtual void Pop(uint32_t ID) = 0;
};
