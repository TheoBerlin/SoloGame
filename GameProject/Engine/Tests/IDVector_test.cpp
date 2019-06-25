#include <catch/catch.hpp>

#include <Engine/Utils/IDVector.hpp>

TEST_CASE("IDVector") {
    IDVector<size_t> idVec;

    size_t elements[] = {3, 4, 6, 1, 9};
    size_t ids[] = {9, 2, 6, 5, 1};

    for (size_t i = 0; i < 5; i += 1) {
        idVec.push_back(elements[i], ids[i]);
    }

    for (size_t i = 0; i < 5; i += 1) {
        // Require that the simple direct indexing returns the correct value
        REQUIRE(idVec[i] == elements[i]);

        // Require that vector elements are indexible by ID
        REQUIRE(idVec.indexID(ids[i]) == elements[i]);

        REQUIRE(idVec.hasElement(ids[i]) == true);
    }

    REQUIRE(idVec.hasElement(1000) == false);

    SECTION("ID link is intact after popping") {
        idVec.pop(ids[0]);

        REQUIRE(idVec.hasElement(ids[0]) == false);

        for (size_t i = 1; i < 5; i += 1) {
            REQUIRE(idVec.indexID(ids[i]) == elements[i]);
        }
    }
}
