#include "EntityManager.hpp"

#include <Engine/ECS/Entity.hpp>

EntityManager::EntityManager()
{
}

EntityManager::~EntityManager()
{
    for (size_t i = 0; i < this->entities.size(); i += 1) {
        delete this->entities[i];
    }
}

void EntityManager::update(float dt)
{
    for (size_t i = 0; i < this->entities.size(); i += 1) {
        this->entities[i]->update(dt);
    }
}

Entity* EntityManager::addEntity(const std::string& name)
{
    size_t entityID = this->entities.size();

    this->entities.push_back(new Entity(entityID, name));

    return this->entities.back();
}

bool EntityManager::removeEntity(size_t entityID)
{
    if (entityID >= this->entities.size()) {
        return false;
    }

    // Overwrite the entity with the last entity element
    delete this->entities[entityID];

    if (entityID != this->entities.size() - 1) {
        // The entity is not the last element, replace it with the last one
        this->entities[entityID] = this->entities.back();

        // Update the replacement entity and its components with a new entity ID
        this->entities[entityID]->setID(entityID);
    }

    this->entities.pop_back();

    return true;
}

Entity* EntityManager::getEntity(size_t entityID)
{
    return this->entities[entityID];
}
