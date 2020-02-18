#pragma once

#include <vector>

class IDContainer
{
public:
    virtual bool hasElement(size_t ID) const = 0;

    virtual size_t size() const = 0;
    virtual const std::vector<size_t>& getIDs() const = 0;
    virtual void pop(size_t ID) = 0;
};
