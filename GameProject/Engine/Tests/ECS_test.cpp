#include <catch/catch.hpp>

#include <Engine/ECS/ComponentHandler.hpp>
#include <Engine/ECS/ECSInterface.hpp>
#include <Engine/ECS/Entity.hpp>
#include <Engine/ECS/System.hpp>
#include <Engine/ECS/SystemSubscriber.hpp>
#include <chrono>
#include <DirectXMath.h>

struct Position {
    DirectX::XMFLOAT3 position;
};

std::type_index tid_pos = std::type_index(typeid(Position));

struct Direction {
    DirectX::XMFLOAT3 direction;
};

std::type_index tid_dir = std::type_index(typeid(Direction));

class TransformHandler_test : public ComponentHandler
{
public:
    TransformHandler_test(SystemSubscriber* sysHandler)
        :ComponentHandler({tid_pos, tid_dir}, sysHandler, std::type_index(typeid(TransformHandler_test)))
    {
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
    ForwardMover(ECSInterface* ecs)
        :System(ecs)
    {
        std::type_index tid_transformHandler = std::type_index(typeid(TransformHandler_test));
        ComponentHandler* handler = this->getComponentHandler(tid_transformHandler);
        this->transformHandler = static_cast<TransformHandler_test*>(handler);

        // Subscribe to components
        SystemRegistration sysReg = {
        {
            {{{RW, tid_pos}, {R, tid_dir}}, &entities}
        },
        this
        };

        this->subscribeToComponents(&sysReg);
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

    TransformHandler_test* transformHandler;
};

TEST_CASE("ECS Subscriptions") {
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

    SECTION("Register system before components are created") {
        // Component handlers must be registered before systems
        TransformHandler_test transformHandler(&ecs.systemSubscriber);

        ForwardMover sys(&ecs);

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

    SECTION("Register system after components are created") {
        /*
            Registering a system after registering components should lead to the system being notified about all existing components
            it's subscribing to
        */
        TransformHandler_test transformHandler(&ecs.systemSubscriber);

        // With this setup, registering the forward mover system should lead to entities[0] being processed
        transformHandler.createPos(pos, entities[0]);
        transformHandler.createDir(dir, entities[0]);

        transformHandler.createDir(dir, entities[1]);

        ForwardMover forwardMover(&ecs);

        const IDVector<Entity>& sysEntities = forwardMover.getEntities();

        REQUIRE(sysEntities.size() == 1);
        REQUIRE(sysEntities[(size_t)0] == entities[(size_t)0]);
    }
}

// The classes and structs below are made for performance testing
struct Power {
    float base;
    float exponent;
};

std::type_index tid_power = std::type_index(typeid(Power));

struct Division {
    float dividend;
    float divisor;
};

std::type_index tid_division = std::type_index(typeid(Division));

class MathComponentHandler : public ComponentHandler
{
public:
    MathComponentHandler(SystemSubscriber* sysSubscriber)
        :ComponentHandler({tid_power, tid_division}, sysSubscriber, std::type_index(typeid(MathComponentHandler)))
    {
        std::vector<ComponentRegistration> compRegs = {
            {tid_power, [this](Entity entity) { return this->powers.hasElement(entity);}, &this->powers.getIDs()},
            {tid_division, [this](Entity entity) { return this->divisions.hasElement(entity);}, &this->divisions.getIDs()}
        };

        this->registerHandler(&compRegs);
    }

    IDVector<Power> powers;
    IDVector<Division> divisions;

    void createPower(Power power, Entity entityID)
    {
        powers.push_back(power, entityID);

        this->registerComponent(tid_power, entityID);
    }

    void createDivision(Division division, Entity entityID)
    {
        divisions.push_back(division, entityID);

        this->registerComponent(tid_division, entityID);
    }
};

// Does a lot of calculations per component to avoid memory bandwidth being the performance bottleneck
class ExpensiveCalculator1 : public System {
public:
    ExpensiveCalculator1(ECSInterface* ecs)
        :System(ecs)
    {
        std::type_index tid_mathCompHandler = std::type_index(typeid(MathComponentHandler));
        ComponentHandler* handler = this->getComponentHandler(tid_mathCompHandler);
        this->mathCompHandler = static_cast<MathComponentHandler*>(handler);

        // Subscribe to components
        SystemRegistration sysReg = {
        {
            {{{RW, tid_power}}, &entities}
        },
        this
        };

        this->subscribeToComponents(&sysReg);
        this->registerUpdate(&sysReg);
    }

    ~ExpensiveCalculator1()
    {
        this->unsubscribeFromComponents({tid_power});
    }

    void update(float dt)
    {
        for (size_t i = 0; i < entities.size(); i += 1) {
            Power& power = mathCompHandler->powers.indexID(entities[i]);

            // Perform power calculations a bunch of times
            for (size_t i = 0; i < 10000; i += 1) {
                power.base += std::powf(power.base, power.exponent) * dt;
            }
        }
    }

private:
    IDVector<Entity> entities;

    MathComponentHandler* mathCompHandler;
};

class ExpensiveCalculator2 : public System {
public:
    ExpensiveCalculator2(ECSInterface* ecs)
        :System(ecs)
    {
        std::type_index tid_mathCompHandler = std::type_index(typeid(MathComponentHandler));
        ComponentHandler* handler = this->getComponentHandler(tid_mathCompHandler);
        this->mathCompHandler = static_cast<MathComponentHandler*>(handler);

        // Subscribe to components
        SystemRegistration sysReg = {
        {
            {{{RW, tid_division}}, &entities}
        },
        this
        };

        this->subscribeToComponents(&sysReg);
        this->registerUpdate(&sysReg);
    }

    ~ExpensiveCalculator2()
    {
        this->unsubscribeFromComponents({tid_division});
    }

    void update(float dt)
    {
        for (size_t i = 0; i < entities.size(); i += 1) {
            Division& division = mathCompHandler->divisions.indexID(entities[i]);

            // Perform power calculations a bunch of times
            for (size_t i = 0; i < 10000; i += 1) {
                division.dividend += dt * division.dividend / division.divisor;
            }
        }
    }

private:
    IDVector<Entity> entities;

    MathComponentHandler* mathCompHandler;
};

double testUpdate(bool multithreaded);

TEST_CASE("ECS System Updates") {
    // Compare the time elapsed when updating with one thread to updating with two threads
    double timeMT = testUpdate(true);
    double timeST = testUpdate(false);

    CAPTURE("Duration multi threaded: %f", timeMT);
    CAPTURE("Duration single threaded: %f", timeST);
    REQUIRE(timeMT < timeST);
}

double testUpdate(bool multithreaded)
{
    ECSInterface ecs;

    MathComponentHandler mathCompHandler(&ecs.systemSubscriber);

    ExpensiveCalculator1 calcSys1(&ecs);
    ExpensiveCalculator2 calcSys2(&ecs);

    // Prepare lots of components for the systems
    for (size_t i = 0; i < 10000; i += 1) {
        Entity entity = ecs.entityIDGen.genID();

        mathCompHandler.createPower({2.0f, 1.1f}, entity);
        mathCompHandler.createDivision({200000.3f, 1.1f}, entity);
    }

    // TODO: Create a simpler interface for measuring and printing time with chrono
    auto start = std::chrono::high_resolution_clock::now();
    if (multithreaded) {
        ecs.systemUpdater.updateMT(0.1f);
    } else {
        ecs.systemUpdater.updateST(0.1f);
    }

    auto end = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double> elapsedTime = end - start;
    return elapsedTime.count();
}
