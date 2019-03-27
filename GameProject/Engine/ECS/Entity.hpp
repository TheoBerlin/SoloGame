#pragma once

#include <string>
#include <vector>

class Component;

class Entity
{
public:
    Entity(size_t ID, const std::string name);
    ~Entity();

    void update(float dt);

    void addComponent(Component* component);
    Component* getComponent(const std::string componentName) const;
    bool removeComponent(const std::string componentName);

    // Unlists all components without deleting them
    void detachAllComponents();

    size_t getID() const;
    void setID(size_t ID);

private:
    std::vector<Component*> components;

    // Index into entity manager's entity array
    size_t ID;
    std::string name;
};
