#include <catch/catch.hpp>
#include <Engine/ECS/ECSInterface.hpp>
#include <Engine/Rendering/AssetLoaders/ModelLoader.hpp>
#include <Engine/Rendering/AssetLoaders/TextureLoader.hpp>
#include <Engine/Rendering/AssetContainers/Model.hpp>
#include <Engine/Rendering/Display.hpp>
#include <Engine/Rendering/ShaderResourceHandler.hpp>

TEST_CASE("ModelLoader") {
    Display display(nullptr, 720, 16.0f/9.0f, true);

    ECSInterface ecs;

    ShaderResourceHandler shaderResourceHandler(&ecs.systemSubscriber, display.getDevice());
    TextureLoader txLoader(&ecs.systemSubscriber, display.getDevice());
    ModelLoader modelLoader(&ecs.systemSubscriber, &txLoader);

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
