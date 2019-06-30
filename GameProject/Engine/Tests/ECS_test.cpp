#include <catch/catch.hpp>

#include <Engine/ECS/ComponentHandler.hpp>
#include <Engine/ECS/ECSInterface.hpp>
#include <Engine/ECS/Entity.hpp>
#include <Engine/ECS/System.hpp>
#include <Engine/ECS/SystemSubscriber.hpp>
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
    TransformHandler(SystemSubscriber* sysHandler)
        :ComponentHandler({tid_pos, tid_dir}, sysHandler)
    {
        std::function<bool(Entity)> testFunc = [this](Entity entity) { return this->positions.hasElement(entity);};

        testFunc(0);
        std::vector<ComponentRegistration> compRegs = {
            {tid_pos, [this](Entity entity) { return this->positions.hasElement(entity);}, &this->positions.getIDs()},
            {tid_dir, [this](Entity entity) { return this->directions.hasElement(entity);}, &this->directions.getIDs()}
        };

        this->registerHandler(&compRegs);
    }

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
    ForwardMover(SystemSubscriber* sysSubscriber)
        :System(sysSubscriber)
    {
        ComponentHandler* handler = this->getComponentHandler(tid_pos);
        this->transformHandler = static_cast<TransformHandler*>(handler);

        // Subscribe to components
        std::vector<ComponentSubReq> subRequests = {
            {{tid_pos, tid_dir}, &entities}
        };

        this->subscribeToComponents(&subRequests);
    }

    ~ForwardMover()
    {
        this->unsubscribeFromComponents({tid_pos, tid_dir});
    }

    void update(float dt)
    {
        for (size_t i = 0; i < entities.size(); i += 1) {
            Position& position = transformHandler->positions.indexID(entities[i]);
            DirectX::XMFLOAT3& pos = transformHandler->positions.indexID(entities[i]).position;
            DirectX::XMFLOAT3& dir = transformHandler->directions.indexID(entities[i]).direction;

            pos.x += dir.x * dt;
            pos.y += dir.y * dt;
            pos.z += dir.z * dt;
        }
    }

    // Used for testing
    const IDVector<Entity>& getEntities() const
    {
        return this->entities;
    }

private:
    IDVector<Entity> entities;

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
        TransformHandler transformHandler(&ecs.systemSubscriber);

        ForwardMover sys(&ecs.systemSubscriber);

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
        TransformHandler transformHandler(&ecs.systemSubscriber);

        // With this setup, registering the forward mover system should lead to entities[0] being processed
        transformHandler.createPos(pos, entities[0]);
        transformHandler.createDir(dir, entities[0]);

        transformHandler.createDir(dir, entities[1]);

        ForwardMover forwardMover(&ecs.systemSubscriber);

        const IDVector<Entity>& sysEntities = forwardMover.getEntities();

        REQUIRE(sysEntities.size() == 1);
        REQUIRE(sysEntities[(size_t)0] == entities[(size_t)0]);
    }
}
