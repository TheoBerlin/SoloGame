#include "MainMenu.hpp"

#include <Engine/Audio/SoundHandler.hpp>
#include <Engine/ECS/ECSCore.hpp>
#include <Engine/GameState/StateManager.hpp>
#include <Engine/InputHandler.hpp>
#include <Engine/Rendering/AssetLoaders/TextureLoader.hpp>
#include <Engine/Rendering/Text/TextRenderer.hpp>
#include <Engine/UI/Panel.hpp>
#include <Engine/Utils/ECSUtils.hpp>
#include <Engine/Utils/Logger.hpp>
#include <Game/States/GameSession.hpp>

MainMenu::MainMenu(StateManager* stateManager, ECSCore* pECS, ID3D11Device* device)
    :State(stateManager, pECS, STATE_TRANSITION::PUSH),
    device(device)
{
    SystemSubscriber* pSystemSubscriber = pECS->getSystemSubscriber();

    this->inputHandler = static_cast<InputHandler*>(pSystemSubscriber->getComponentHandler(TID(InputHandler)));
    inputHandler->setMouseMode(DirectX::Mouse::Mode::MODE_ABSOLUTE);
    inputHandler->setMouseVisibility(true);
    this->keyboardState = inputHandler->getKeyboardState();

    UIHandler* uiHandler            = static_cast<UIHandler*>(pSystemSubscriber->getComponentHandler(TID(UIHandler)));
    TextRenderer* pTextRenderer     = static_cast<TextRenderer*>(pSystemSubscriber->getComponentHandler(TID(TextRenderer)));
    TextureLoader* pTextureLoader   = static_cast<TextureLoader*>(pSystemSubscriber->getComponentHandler(TID(TextureLoader)));

    // Create UI panel
    uiEntity = m_pECS->createEntity();
    uiHandler->createPanel(uiEntity, {0.4f, 0.45f}, {0.2f, 0.1f}, {0.0f, 0.0f, 0.0f, 0.0f}, 1.0f);

    // Attach background and text textures to the panel
    TextureReference panelTextures[2] = {
        pTextureLoader->loadTexture("./Game/Assets/Models/Cube.png"),
        pTextRenderer->renderText("Play", "Game/Assets/Fonts/arial/arial.ttf", 50)
    };

    TextureAttachmentInfo txAttachmentInfos[2];
    txAttachmentInfos[0].sizeSetting            = TX_SIZE_STRETCH;
    txAttachmentInfos[1].horizontalAlignment    = TX_HORIZONTAL_ALIGNMENT_CENTER;
    txAttachmentInfos[1].verticalAlignment      = TX_VERTICAL_ALIGNMENT_CENTER;
    txAttachmentInfos[1].sizeSetting            = TX_SIZE_CLIENT_RESOLUTION_DEPENDENT;

    uiHandler->attachTextures(uiEntity, txAttachmentInfos, panelTextures, ARRAYSIZE(panelTextures));

    // Make the panel a button
    uiHandler->createButton(uiEntity, {0.1f, 0.0f, 0.0f, 1.0f}, {0.2f, 0.0f, 0.0f, 1.0f}, [this](){ new GameSession(this); });

    // Create test sound
    Entity soundEntity = m_pECS->createEntity();
    const std::string soundFile = "./Game/Assets/Sounds/muscle-car-daniel_simon.mp3";

    SoundHandler* pSoundHandler = reinterpret_cast<SoundHandler*>(pSystemSubscriber->getComponentHandler(TID(SoundHandler)));
    if (pSoundHandler->createSound(soundEntity, soundFile)) {
        pSoundHandler->playSound(soundEntity);
    }

    LOG_INFO("Entered main menu, press E to start a game session");
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
        new GameSession(this);
    }
}

ID3D11Device* MainMenu::getDevice()
{
    return device;
}
