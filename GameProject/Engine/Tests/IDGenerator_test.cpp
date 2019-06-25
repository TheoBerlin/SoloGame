#include <catch/catch.hpp>

#include <Engine/Utils/IDGenerator.hpp>

TEST_CASE("IDGenerator") {
    IDGenerator idGen;

    for (size_t i = 0; i < 3; i += 1) {
        REQUIRE(i == idGen.genID());
    }

    // Require that IDs can be popped and recycled
    idGen.popID(0);
    idGen.popID(1);
    REQUIRE(idGen.genID() == 0);
    REQUIRE(idGen.genID() == 1);

    // Require that the generator keeps escalating the generated IDs once there is nothing to recycle
    REQUIRE(idGen.genID() == 3);
}
