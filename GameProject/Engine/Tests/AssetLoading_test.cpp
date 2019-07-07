#include <catch/catch.hpp>
#include <Engine/Rendering/AssetLoaders/ModelLoader.hpp>
#include <Engine/Rendering/AssetLoaders/TextureLoader.hpp>
#include <Engine/Rendering/AssetContainers/Model.hpp>

TEST_CASE("ModelLoader") {
    Model* umbrellaModel = ModelLoader::loadModel("./Game/Assets/Models/untitled.dae",
    std::vector<TX_TYPE>(1, TX_TYPE::DIFFUSE));

    // Check that the model was loaded
    REQUIRE(umbrellaModel != nullptr);

    // Check that the model contains mesh
    std::vector<Mesh>& meshes = umbrellaModel->meshes;

    REQUIRE(meshes.size() > 0);

    // Check that each mesh contains vertices
    for (size_t i = 0; i < meshes.size(); i += 1) {
        REQUIRE(meshes[i].vertices.size() > 0);
    }

    Model* duplicateAttempt = ModelLoader::loadModel("./Game/Assets/Models/untitled.dae",
    std::vector<TX_TYPE>(1, TX_TYPE::DIFFUSE));

    // Check that the model was not loaded twice
    REQUIRE(umbrellaModel == duplicateAttempt);

    ModelLoader::deleteAllModels();
    TextureLoader::deleteAllTextures();
}
