#include <catch/catch.hpp>
#include <Engine/ECS/ECSCore.hpp>
#include <Engine/Rendering/AssetLoaders/ModelLoader.hpp>
#include <Engine/Rendering/AssetLoaders/TextureLoader.hpp>
#include <Engine/Rendering/AssetContainers/Model.hpp>
#include <Engine/Rendering/Display.hpp>
#include <Engine/Rendering/ShaderResourceHandler.hpp>

TEST_CASE("ModelLoader") {
    Display display(nullptr, 720, 16.0f/9.0f, true);

    ECSCore ecs;

    ShaderResourceHandler shaderResourceHandler(ecs.getSystemSubscriber(), display.getDevice());
    TextureLoader txLoader(ecs.getSystemSubscriber(), display.getDevice());
    ModelLoader modelLoader(ecs.getSystemSubscriber(), &txLoader);

    Model* umbrellaModel = modelLoader.loadModel("./Game/Assets/Models/Cube.dae");

    // Check that the model was loaded
    REQUIRE(umbrellaModel != nullptr);

    // Check that the model contains mesh
    std::vector<Mesh>& meshes = umbrellaModel->meshes;

    REQUIRE(meshes.size() > 0);

    // Check that each mesh contains a vertex buffer
    for (size_t i = 0; i < meshes.size(); i += 1) {
        REQUIRE(meshes[i].vertexBuffer != nullptr);
    }

    Model* duplicateAttempt = modelLoader.loadModel("./Game/Assets/Models/Cube.dae");

    // Check that the model was not loaded twice
    REQUIRE(umbrellaModel == duplicateAttempt);
}
