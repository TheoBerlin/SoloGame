#pragma once

#include <Engine/ECS/Entity.hpp>
#include <vector>

class EntityManager
{
public:
    EntityManager();
    ~EntityManager();

    void update(float dt);

    Entity* addEntity(const std::string& name);
    bool removeEntity(size_t entityID);

    Entity* getEntity(size_t entityID);

private:
    std::vector<Entity*> entities;
};
