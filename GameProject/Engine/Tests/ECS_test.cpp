#include <catch/catch.hpp>

#include <Engine/ECS/Component.hpp>
#include <Engine/ECS/Entity.hpp>
#include <Engine/ECS/EntityManager.hpp>

class TestComponent : public Component {
public:
    TestComponent(Entity* host, unsigned n) : Component("TestComponent", host), n(n) {};

    void update(float dt) { n += 1; };

    unsigned int n;
};

TEST_CASE("EntityManager") {
    EntityManager* em = new EntityManager();

    // Create entity and attach test component
    Entity* testEntity = em->addEntity("TestEntity");
    TestComponent* testComponent = new TestComponent(testEntity, 0);

    // Make sure that entities update their components
    testEntity->update(0.0f);

    REQUIRE(testComponent->n == 1);

    // Make sure that the entity manager updates its entities
    em->update(0.0f);

    REQUIRE(testComponent->n == 2);

    // Add a second entity
    Entity* secondEntity = em->addEntity("SecondEntity");
    TestComponent* secondComponent = new TestComponent(secondEntity, 0);

    // Delete the first entity
    em->removeEntity(testEntity->getID());

    // The second entity should now be the first element
    REQUIRE(secondEntity->getID() == 0);

    em->update(0.0f);

    // Make sure that the second entity is still updateable after a deletion
    REQUIRE(secondComponent->n == 1);

    delete em;
}
