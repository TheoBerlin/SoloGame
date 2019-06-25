#include <catch/catch.hpp>

#include <Engine/ECS/ComponentHandler.hpp>
#include <Engine/ECS/ECSInterface.hpp>
#include <Engine/ECS/Entity.hpp>
#include <Engine/ECS/System.hpp>
#include <Engine/ECS/SystemHandler.hpp>
#include <DirectXMath.h>

struct Position {
    DirectX::XMFLOAT3 position;
};

std::type_index tid_pos = std::type_index(typeid(Position));

struct Direction {
    DirectX::XMFLOAT3 direction;
};

std::type_index tid_dir = std::type_index(typeid(Direction));

class TransformHandler : public ComponentHandler
{
public:
    TransformHandler(SystemHandler* sysHandler)
        :ComponentHandler({tid_pos, tid_dir}, sysHandler)
    {}

    IDVector<Position> positions;
    IDVector<Direction> directions;

    void createPos(Position pos, Entity entityID)
    {
        positions.push_back(pos, entityID);

        this->registerComponent(tid_pos, entityID);
    }

    void createDir(Direction dir, Entity entityID)
    {
        directions.push_back(dir, entityID);

        this->registerComponent(tid_dir, entityID);
    }

    std::vector<Entity> getEntities(std::type_index componentType)
    {
        if (componentType == tid_pos) {
            return positions.getIDs();
        }

        return directions.getIDs();
    }
};

class ForwardMover : public System {
public:
    ForwardMover(SystemHandler* sysHandler)
        :System(sysHandler)
    {
        this->componentTypes = {tid_pos, tid_dir};

        ComponentHandler* handler = this->getComponentHandler(tid_pos);
        this->transformHandler = static_cast<TransformHandler*>(handler);

        this->subscribeToComponents();
    }

    void update(float dt)
    {
        for (size_t i = 0; i < entityIDs.size(); i += 1) {
            Position& position = transformHandler->positions.indexID(entityIDs[i]);
            DirectX::XMFLOAT3& pos = transformHandler->positions.indexID(entityIDs[i]).position;
            DirectX::XMFLOAT3& dir = transformHandler->directions.indexID(entityIDs[i]).direction;

            pos.x += dir.x * dt;
            pos.y += dir.y * dt;
            pos.z += dir.z * dt;
        }
    }

    void newComponent(Entity entityID, std::type_index componentType)
    {
        if (transformHandler->positions.hasElement(entityID) && transformHandler->directions.hasElement(entityID)) {
            entityIDs.push_back(entityID, entityID);
        }
    }

    void removedComponent(Entity entityID, std::type_index componentType)
    {
        entityIDs.pop(entityID);
    }

    const IDVector<Entity>& getEntities() const
    {
        return this->entityIDs;
    }

private:
    TransformHandler* transformHandler;
};

TEST_CASE("ECS") {
    /*
        This set of unit tests serves as a proof of concept and a handbook on how to use the ECS implementation
    */
    // Initial ECS and test setup
    ECSInterface ecs;

    const size_t entityCount = 2;
    Entity entities[entityCount];

    for(size_t i = 0; i < entityCount; i += 1) {
        entities[i] = ecs.entityIDGen.genID();
    }

    Position pos;
    pos.position = {0.0f, 0.0f, 0.0f};
    Direction dir;
    dir.direction = {0.0f, 0.0f, 1.0f};

    SECTION("Component subscriptions") {
        // Component handlers must be registered before systems
        TransformHandler transformHandler(&ecs.systemHandler);

        ForwardMover sys(&ecs.systemHandler);

        /*
            The forward mover system requires both a position and a direction component, require that
            the system does not process anything until an entity with both components exists
        */
        transformHandler.createPos(pos, entities[0]);
        transformHandler.createDir(dir, entities[1]);
        REQUIRE(sys.getEntities().size() == 0);

        // Register components so that the system should start processing them
        transformHandler.createDir(dir, entities[0]);
        REQUIRE(sys.getEntities().size() == 1);

        transformHandler.createPos(pos, entities[1]);
        REQUIRE(sys.getEntities().size() == 2);
    }

    SECTION("Register system after components") {
        /*
            Registering a system after registering components should lead to the system being notified about all existing components
            it's subscribing to
        */
        TransformHandler transformHandler(&ecs.systemHandler);

        // With this setup, registering the forward mover system should lead to entities[0] being processed
        transformHandler.createPos(pos, entities[0]);
        transformHandler.createDir(dir, entities[0]);

        transformHandler.createDir(dir, entities[1]);

        ForwardMover forwardMover(&ecs.systemHandler);

        const IDVector<Entity>& sysEntities = forwardMover.getEntities();

        REQUIRE(sysEntities.size() == 1);
        REQUIRE(sysEntities[(size_t)0] == entities[(size_t)0]);
    }
}
