#pragma once

#include <string>

class Entity;

class Component
{
public:
    Component(const std::string name, Entity* host);
    virtual ~Component();

    virtual void update(float dt) = 0;

    void setEntityID(size_t entityID);

    const std::string& getName() const;

private:
    size_t entityID;

    std::string name;
};
