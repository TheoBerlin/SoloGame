#include <catch/catch.hpp>
#include <Engine/Rendering/AssetLoaders/ModelLoader.hpp>
#include <Engine/Rendering/AssetContainers/Model.hpp>

TEST_CASE("ModelLoader") {
    Model* umbrellaModel = ModelLoader::loadModel("./Game/Assets/Models/Cube.fbx",
    std::vector<aiTextureType>(1, aiTextureType_DIFFUSE));

    // Check that the model was loaded
    REQUIRE(umbrellaModel != nullptr);

    Model* duplicateAttempt = ModelLoader::loadModel("./Game/Assets/Models/Cube.fbx",
    std::vector<aiTextureType>(1, aiTextureType_DIFFUSE));

    // Check that the model was not loaded twice
    REQUIRE(umbrellaModel == duplicateAttempt);

    // Check that the model contains mesh
    std::vector<Mesh>& meshes = umbrellaModel->getMeshes();

    REQUIRE(meshes.size() > 0);

    // Check that each mesh contains vertices
    for (size_t i = 0; i < meshes.size(); i += 1) {
        REQUIRE(meshes[i].getVertices().size() > 0);
    }
}
