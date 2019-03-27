#include "Component.hpp"

#include <Engine/ECS/Entity.hpp>

Component::Component(const std::string name, Entity* host)
    :name(name),
    entityID(host->getID())
{
    host->addComponent(this);
}

Component::~Component()
{
}

void Component::setEntityID(size_t entityID)
{
    this->entityID = entityID;
}

const std::string& Component::getName() const
{
    return this->name;
}
