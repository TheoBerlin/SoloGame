#include "MainMenu.hpp"

#include <Engine/ECS/ECSInterface.hpp>
#include <Engine/GameState/StateManager.hpp>
#include <Engine/InputHandler.hpp>
#include <Engine/Rendering/AssetLoaders/TextureLoader.hpp>
#include <Engine/Rendering/Text/TextRenderer.hpp>
#include <Engine/UI/Panel.hpp>
#include <Engine/Utils/Logger.hpp>
#include <Game/States/GameSession.hpp>

MainMenu::MainMenu(StateManager* stateManager, ECSInterface* ecs, ID3D11Device* device)
    :State(stateManager, ecs),
    device(device)
{
    std::type_index tid_inputHandler = std::type_index(typeid(InputHandler));

    this->inputHandler = static_cast<InputHandler*>(ecs->systemSubscriber.getComponentHandler(tid_inputHandler));
    inputHandler->setMouseMode(DirectX::Mouse::Mode::MODE_ABSOLUTE);
    inputHandler->setMouseVisibility(true);
    this->keyboardState = inputHandler->getKeyboardState();

    std::type_index tid_uiHandler = std::type_index(typeid(UIHandler));
    UIHandler* uiHandler = static_cast<UIHandler*>(ecs->systemSubscriber.getComponentHandler(tid_uiHandler));
    std::type_index tid_textRenderer = std::type_index(typeid(TextRenderer));
    TextRenderer* pTextRenderer = static_cast<TextRenderer*>(ecs->systemSubscriber.getComponentHandler(tid_textRenderer));
    std::type_index tid_textureLoader = std::type_index(typeid(TextureLoader));
    TextureLoader* pTextureLoader = static_cast<TextureLoader*>(ecs->systemSubscriber.getComponentHandler(tid_textureLoader));

    // Create UI panel
    uiEntity = ecs->entityIDGen.genID();
    uiHandler->createPanel(uiEntity, {0.4f, 0.45f}, {0.2f, 0.1f}, {0.0f, 0.0f, 0.0f, 0.0f}, 1.0f);

    // Attach background and text textures to the panel
    TextureReference panelTextures[2] = {
        pTextureLoader->loadTexture("./Game/Assets/Models/Cube.png"),
        pTextRenderer->renderText("Play", "Game/Assets/Fonts/arial/arial.ttf", 50)
    };

    TextureAttachmentInfo txAttachmentInfos[2];
    txAttachmentInfos[0].sizeSetting = TX_SIZE_STRETCH;
    txAttachmentInfos[1].horizontalAlignment = TX_HORIZONTAL_ALIGNMENT_CENTER;
    txAttachmentInfos[1].verticalAlignment = TX_VERTICAL_ALIGNMENT_CENTER;
    txAttachmentInfos[1].sizeSetting = TX_SIZE_CLIENT_RESOLUTION_DEPENDENT;

    uiHandler->attachTextures(uiEntity, txAttachmentInfos, panelTextures, ARRAYSIZE(panelTextures));

    // Make the panel a button
    uiHandler->createButton(uiEntity, {0.1f, 0.0f, 0.0f, 1.0f}, {0.2f, 0.0f, 0.0f, 1.0f}, [this](){this->stateManager->pushState(new GameSession(this));});

    Logger::LOG_INFO("Entered main menu, press E to start a game session");
}

MainMenu::~MainMenu()
{}

void MainMenu::resume()
{
    inputHandler->setMouseMode(DirectX::Mouse::Mode::MODE_ABSOLUTE);
    inputHandler->setMouseVisibility(true);
}

void MainMenu::pause()
{}

void MainMenu::update(float dt)
{
    if (keyboardState->E) {
        this->stateManager->pushState(new GameSession(this));
    }
}

ID3D11Device* MainMenu::getDevice()
{
    return device;
}
