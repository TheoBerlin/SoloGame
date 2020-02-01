#include "MainMenu.hpp"

#include <Engine/ECS/ECSInterface.hpp>
#include <Engine/GameState/StateManager.hpp>
#include <Engine/InputHandler.hpp>
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

    // Create UI panel
    uiEntity = ecs->entityIDGen.genID();
    uiHandler->createPanel(uiEntity, {0.4f, 0.45f}, {0.2f, 0.1f}, {0.0f, 0.0f, 0.0f, 0.0f}, 0.05f);

    // Attach texture to the panel
    TextureAttachmentInfo txAttachmentInfo = {};
    txAttachmentInfo.sizeSetting = TX_SIZE_STRETCH;
    uiHandler->attachTexture(uiEntity, txAttachmentInfo, "./Game/Assets/Models/Cube.png");

    // Attach text to the panel
    txAttachmentInfo.horizontalAlignment = TX_HORIZONTAL_ALIGNMENT_CENTER;
    txAttachmentInfo.verticalAlignment = TX_VERTICAL_ALIGNMENT_CENTER;
    txAttachmentInfo.sizeSetting = TX_SIZE_CLIENT_RESOLUTION_DEPENDENT;
    ID3D11ShaderResourceView* textSRV = pTextRenderer->renderText("Play", "Game/Assets/Fonts/arial/arial.ttf", 50);
    uiHandler->attachTexture(uiEntity, txAttachmentInfo, textSRV);

    // Make the panel a button
    uiHandler->createButton(uiEntity, {0.7f, 0.0f, 0.0f, 0.0f}, {1.0f, 0.0f, 0.0f, 0.0f}, [this](){this->stateManager->pushState(new GameSession(this));});

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
