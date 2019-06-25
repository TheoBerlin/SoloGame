#pragma once

#include <queue>

class IDGenerator
{
public:
    IDGenerator();
    ~IDGenerator();

    size_t genID();

    // Registers ID as free to be generated again
    void popID(size_t ID);

private:
    size_t nextFree;

    std::queue<size_t> recycled;
};
