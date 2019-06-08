#include "IDGenerator.hpp"

IDGenerator::IDGenerator()
    :nextFree(0)
{}

IDGenerator::~IDGenerator()
{}

size_t IDGenerator::genID()
{
    if (!recycled.empty()) {
        size_t newID = recycled.back();
        recycled.pop();

        return newID;
    }

    return nextFree++;
}

void IDGenerator::popID(size_t ID)
{
    recycled.push(ID);
}
