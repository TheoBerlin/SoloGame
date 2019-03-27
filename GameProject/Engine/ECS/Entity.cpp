#include "Entity.hpp"

#include <Engine/ECS/Component.hpp>

Entity::Entity(size_t ID, const std::string name)
    :ID(ID),
    name(name)
{
}

Entity::~Entity()
{
    for (size_t i = 0; i < components.size(); i += 1) {
        delete this->components[i];
    }
}

void Entity::update(float dt)
{
    for (size_t i = 0; i < components.size(); i += 1) {
        this->components[i]->update(dt);
    }
}

void Entity::addComponent(Component* component)
{
    this->components.push_back(component);
}

Component* Entity::getComponent(const std::string componentName) const
{
    for (size_t i = 0; i < components.size(); i += 1) {
        if (this->components[i]->getName() == componentName) {
            return this->components[i];
        }
    }

    return nullptr;
}

bool Entity::removeComponent(const std::string componentName)
{
    for (size_t i = 0; i < components.size(); i += 1) {
        if (this->components[i]->getName() == componentName) {
           delete this->components[i];

           return true;
        }
    }

    return false;
}

void Entity::detachAllComponents()
{
    this->components.clear();
}

size_t Entity::getID() const
{
    return this->ID;
}

void Entity::setID(size_t ID)
{
    this->ID = ID;

    // Update entity ID in components
    for (unsigned int i = 0; i < this->components.size(); i += 1) {
        this->components[i]->setEntityID(ID);
    }
}
